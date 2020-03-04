/*-------------------------------------------------------------------------*\
This code is part of poreFOAM, a suite of codes written using OpenFOAM
for direct simulation of flow at the pore scale. 	
You can redistribute this code and/or modify this code under the 
terms of the GNU General Public License (GPL) as published by the  
Free Software Foundation, either version 3 of the License, or (at 
your option) any later version. see <http://www.gnu.org/licenses/>.


The code has been developed by Ali Qaseminejad Raeini as a part his PhD 
at Imperial College London, under the supervision of Branko Bijeljic 
and Martin Blunt. 

Please see our website for relavant literature:
http://www3.imperial.ac.uk/earthscienceandengineering/research/perm/porescalemodelling

For further information please contact us by email:
Ali Q Raeini:    a.qaseminejad-raeini09@imperial.ac.uk
Branko Bijeljic: b.bijeljic@imperial.ac.uk
Martin J Blunt:  m.blunt@imperial.ac.uk
 
 Description:
	creates a surface between the pore and the solid from a 3D rock image
\*-------------------------------------------------------------------------*/


void correct( faceList & faces, labelList& fMarks, DynamicField<point> & points, bool handlemultipliConnectedEdges  );
void correctbioti( faceList & faces, labelList& fMarks, DynamicField<point> & points, int stage );
void correctbioti2( faceList & faces, labelList& fMarks, DynamicField<point> & points, int stage );



void writeSTLBINARY( const voxelMesh & data, std::string outputSurface) 
{


    Info<<"writeSTLBINARY: "<<endl;



    double3D dx=data.dx(),X0=data.X0();

    //~ =*this;
    unsigned int n[3],n1, n2, n3;
    data.getSize(n[0],n[1],n[2]);
    data.getPrivateSize(n1,n2,n3);       //opposite order to n[]
    n1-=2; n2-=2; n3-=2;
    Info<<" "<<n1<<", "<<n2<<", "<<n3<<endl;
    Info<<"dx: "<<dx[0]<<", "<<dx[1]<<", "<<dx[2]<<endl;
    Info<<"X0: "<<X0[0]<<", "<<X0[1]<<", "<<X0[2]<<endl;



//=======================================================================




    int nInternalFaces=0;
    int nAllFaces=0;
    int nLeftFaces=0;
    int nRightFaces=0;
    int nBottomFaces=0;
    int nTopFaces=0;
    int nBackFaces=0;
    int nFrontFaces=0;

#define  count3(type) ++n##type ##Faces;
#define ucount3(type) count3(type)
#define  count2(type) count3(type)
#define ucount2(type) count3(type)
#define  count1(type) count3(type)
#define ucount1(type) count3(type)


    for (int kk=1;kk<=n1;kk++)
     for (int jj=1;jj<=n2;jj++)
       for (int ii=1;ii<=n3;ii++)
    	{
			unsigned char vv=data[kk][jj][ii];
             if (vv==1 || vv==2)
             {                
				if (ii!=1)                                                                            
				{                                                                                     
					if (vv==data[kk][jj][ii-1])    {/*count3( Internal);internal[iface]=true;*/}          
					else  if(1!=data[kk][jj][ii-1]) count3( All);
				}//else {count3( Back);}                                     
				if (ii!=n3)                                                               
				{                                                                         
					if (vv==data[kk][jj][ii+1])    {/*ucount3( Internal);internal[iface]=true;*/}      
					else if(1!=data[kk][jj][ii+1]) ucount3( All);
				}//else {ucount3( Front); }                                  


				if (jj!=1)                                                            
				{                                                                     
					if (vv==data[kk][jj-1][ii])    {/*count2( Internal);internal[iface]=true;*/}       
					else if(1!=data[kk][jj-1][ii]) count2( All);
				}//else {count2( Bottom)}                                
				if (jj!=n2) 
				{                                                                     
					if (vv==data[kk][jj+1][ii])    {/*ucount2( Internal);internal[iface]=true;*/}            
					else if(1!=data[kk][jj+1][ii]) ucount2( All);
				}//else {ucount2( Top)}                                  


				if (kk!=1)                                                            
				{                                                                     
					if (vv==data[kk-1][jj][ii])    {/*count1( Internal);internal[iface]=true;*/}       
					else  if(1!=data[kk-1][jj][ii]) count1( All);
				}//else {count1( Left)}                                  
				if (kk!=n1)
				{                                                                     
					if (vv==data[kk+1][jj][ii])    {/*ucount1( Internal);internal[iface]=true;*/}      
					else  if(1!=data[kk+1][jj][ii]) ucount1( All); 
				}//else {ucount1( Right)}        
             }
         }

    Info<<"nInternalFaces: "<<nInternalFaces<<endl;
    Info<<"nAllFaces: "<<nAllFaces<<endl;
    Info<<"nLeftFaces: "<<nLeftFaces<<endl;
    Info<<"nRightFaces: "<<nRightFaces<<endl;
    Info<<"nBottomFaces: "<<nBottomFaces<<endl;
    Info<<"nTopFaces: "<<nTopFaces<<endl;
    Info<<"nBackFaces: "<<nBackFaces<<endl;
    Info<<"nFrontFaces: "<<nFrontFaces<<endl;

        std::cout.flush();



     int iPoints=-1;



voxelField<int> point_mapper(n[0]+1,n[1]+1,n[2]+1,-1);

    DynamicField<point> points;

faceList faces_All(nAllFaces);
labelList fMarks(nAllFaces);
faceList faces_Left(nLeftFaces);
faceList faces_Right(nRightFaces );
faceList faces_Bottom(nBottomFaces);
faceList faces_Top(nTopFaces);
faceList faces_Back(nBackFaces);
faceList faces_Front(nFrontFaces);



#define addPointToFace_m(pointIndex, iii,jjj,kkk ,type)                                 \
    if (point_mapper[iii][jjj][kkk]<0)                                                  \
    {                                                                                   \
        points.append(point(dx[0]*(kkk+1.0)+X0[0],dx[1]*(jjj+1.0)+X0[1],dx[2]*(iii+1.0)+X0[2]));     \
        point_mapper[iii][jjj][kkk]=++iPoints;                                          \
    }                                                                                   \
    faces_##type[i##type ##Faces][pointIndex]=point_mapper[iii][jjj][kkk];



#define recordFaces_m( l10,l11,l20,l21,l30,l31, ii,jj,kk,type )                      \
  {                                                                                  \
          ++i##type##Faces ;                                                        \
          faces_##type[i##type ##Faces].setSize(4);                                  \
          addPointToFace_m(0, ii, jj, kk, type)                                      \
          addPointToFace_m(1, ii+l10, jj+l20, kk+l30, type)                          \
          addPointToFace_m(2, ii+l10+l11, jj+l20+l21, kk+l30+l31, type)              \
          addPointToFace_m(3, ii+l11, jj+l21, kk+l31, type)                          \
  } 


#define  clockwiserecordFaces3(type) recordFaces_m( 0,1,1,0,0,0,  kk-1,jj-1,ii-1, type)
#define uclockwiserecordFaces3(type) recordFaces_m( 1,0,0,1,0,0,  kk-1,jj-1,ii  , type)
#define  clockwiserecordFaces2(type) recordFaces_m( 1,0,0,0,0,1,  kk-1,jj-1,ii-1, type)
#define uclockwiserecordFaces2(type) recordFaces_m( 0,1,0,0,1,0,  kk-1,jj  ,ii-1, type)
#define  clockwiserecordFaces1(type) recordFaces_m( 0,0,0,1,1,0,  kk-1,jj-1,ii-1, type)
#define uclockwiserecordFaces1(type) recordFaces_m( 0,0,1,0,0,1,  kk  ,jj-1,ii-1, type)






    int iCells=-1;


    int iAllFaces=-1;

    for (register unsigned int kk=1;kk<=n1;kk++)
    {
        for (register unsigned int jj=1;jj<=n2;jj++)

        {
            for (register unsigned int ii=1;ii<=n3;ii++)
            {
				unsigned char vv=data[kk][jj][ii];
                if (vv==1 || vv==2)
                {

                    iCells++;                                                       

					if (ii!=1)                                                                            
					{                                                                                     
						if (vv==data[kk][jj][ii-1])    {/*clockwiserecordFaces3( Internal);internal[iface]=true;*/}          
						else  if(1!=data[kk][jj][ii-1]) {clockwiserecordFaces3( All); fMarks[iAllFaces]=vv+data[kk][jj][ii-1]; }
					}//else {clockwiserecordFaces3( Back);}                                     
					if (ii!=n3)                                                               
					{                                                                         
						if (vv==data[kk][jj][ii+1])    {/*uclockwiserecordFaces3( Internal);internal[iface]=true;*/}      
						else if(1!=data[kk][jj][ii+1]){uclockwiserecordFaces3( All); fMarks[iAllFaces]=vv+data[kk][jj][ii+1]; }         
					}//else {uclockwiserecordFaces3( Front); }                                  


					if (jj!=1)                                                            
					{                                                                     
						if (vv==data[kk][jj-1][ii])    {/*clockwiserecordFaces2( Internal);internal[iface]=true;*/}       
						else if(1!=data[kk][jj-1][ii]) {clockwiserecordFaces2( All);  fMarks[iAllFaces]=vv+data[kk][jj-1][ii]; }      
					}//else {clockwiserecordFaces2( Bottom)}                                
					if (jj!=n2)                                                           
					{                                                                     
						if (vv==data[kk][jj+1][ii])    {/*uclockwiserecordFaces2( Internal);internal[iface]=true;*/}            
						else if(1!=data[kk][jj+1][ii]) {uclockwiserecordFaces2( All);  fMarks[iAllFaces]=vv+data[kk][jj+1][ii]; }     
					}//else {uclockwiserecordFaces2( Top)}                                  


					if (kk!=1)                                                            
					{                                                                     
						if (vv==data[kk-1][jj][ii])    {/*clockwiserecordFaces1( Internal);internal[iface]=true;*/}       
						else  if(1!=data[kk-1][jj][ii]) {clockwiserecordFaces1( All); fMarks[iAllFaces]=vv+data[kk-1][jj][ii];}      
					}//else {clockwiserecordFaces1( Left)}                                  
					if (kk!=n1)                                                           
					{                                                                     
						if (vv==data[kk+1][jj][ii])    {/*uclockwiserecordFaces1( Internal);internal[iface]=true;*/}      
						else  if(1!=data[kk+1][jj][ii]) {uclockwiserecordFaces1( All); fMarks[iAllFaces]=vv+data[kk+1][jj][ii];}         
					}//else {uclockwiserecordFaces1( Right)}                                    

                }
            }
        }
    }


//______________________________________________

point_mapper.resize(0);


    Info<<"nPoints: "<<points.size()<<"      "<<endl;        /*Info.flush()*/;


	correct(faces_All,fMarks,points,true);
	correct(faces_All,fMarks,points,true);
	correct(faces_All,fMarks,points,true);
	correct(faces_All,fMarks,points,true);
	correct(faces_All,fMarks,points,false);
	correct(faces_All,fMarks,points,false);
	correct(faces_All,fMarks,points,false);
	correct(faces_All,fMarks,points,false);


Info<<1<<endl;
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,0);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,1);
	correctbioti(faces_All,fMarks,points,1);

Info<<2<<endl;


Info<<endl;



    {

		faceList facesSorted_All(faces_All.size());
		labelList zoneSizes(3);
		
		int indF=-1;
		for (int i=0; i<faces_All.size(); ++i)
			if(fMarks[i]==1) facesSorted_All[++indF]=faces_All[i];
		zoneSizes[0]=indF+1; 
		for (int i=0; i<faces_All.size(); ++i)
			if(fMarks[i]==2) facesSorted_All[++indF]=faces_All[i];
		zoneSizes[1]=indF+1-zoneSizes[0]; 
		for (int i=0; i<faces_All.size(); ++i)
			if(fMarks[i]==3) facesSorted_All[++indF]=faces_All[i];
			else if(fMarks[i]>3) Info<<" Error wrong  label in the image"<<endl;
		zoneSizes[2]=indF+1-zoneSizes[1]-zoneSizes[0]; 

		Info<<faces_All.size()<<" ?= "<<facesSorted_All.size()<<endl;
		Info<<zoneSizes[0]<<" + "<<zoneSizes[1]<<" + "<<zoneSizes[2]<<" = "<<indF+1<<endl;
    
    
        Xfer<List<face> > facesFer(facesSorted_All,true);
        Field<point> & pointsSF=points;
        Xfer<Field<point> > pointsFer(pointsSF,true);
        meshedSurface surf1(pointsFer, facesFer, zoneSizes);
		//~ surf1.addZones();
        //~ surf.triangulate ();

        surf1.write(outputSurface);

        
        //~ std::ofstream  offile("fMarks.txt");
        //~ for (int i=0; i<fMarks.size(); ++i)		offile<<fMarks[i]<<"  ";
		//~ offile.close();
    }

}


int appendUnique(DynamicList<label>& dynList, label value)
{
    forAll(dynList,i) if(dynList[i]==value) return 0;
	dynList.append(value);
	return 1;
}

int getMinPos(const UList<scalar>& array)
{
    scalar min=array[0];
    int minPos=0;
    for (int i=1; i<array.size(); i++)   if(min>array[i])  { min=array[i];  minPos=i; }
    return minPos;
}

int findPos(const UList<label>& list,label value)
{
    forAll(list,i)    if (value==list[i])    return i;
    return -1;
}


inline int appendSinglyConnectedNeis(label meshPI ,DynamicList<label> & group1, const meshedSurface& surf1, labelList& fMarks, bool handlemultipliConnectedEdges)
{
    const bool selectMin=false;
    int nCollected=0;

	const labelListList& pEdges = surf1.pointEdges();
	const labelListList& eFaces = surf1.edgeFaces();
	const List<edge>& edges = surf1.edges();
	const List<face>& faces = surf1.faces();
	const pointField& points = surf1.points();
	const labelList& meshPoints = surf1.meshPoints();
  forAll(group1, gfI)
  {
	label connectingFace=group1[gfI];
//-----------------------------------------------

    const labelList & myEdges=pEdges[meshPI];
    forAll(myEdges, myeI)
    {
        label eI=myEdges[myeI];

        const labelList & myEdgeFaces=eFaces[eI];

        if (myEdgeFaces.size()==2)
        {
            if      (myEdgeFaces[0]==connectingFace) nCollected+=appendUnique(group1,myEdgeFaces[1]);
            else if (myEdgeFaces[1]==connectingFace) nCollected+=appendUnique(group1,myEdgeFaces[0]);
        }
        else if (myEdgeFaces.size()>2)
        {
            bool isMyEdge =false; label cnctedMark=fMarks[connectingFace]; label otherFI=-1; label anotherFI=-1;
            forAll(myEdgeFaces, fI)
            {
				if(fMarks[myEdgeFaces[fI]] != cnctedMark)
				{	if (otherFI==-1)  otherFI = myEdgeFaces[fI];
					else if(fMarks[myEdgeFaces[fI]] != fMarks[otherFI])  anotherFI = myEdgeFaces[fI];
				}
				else  if (myEdgeFaces[fI]==connectingFace) isMyEdge=true;
			}
			
			if (otherFI>=0 && anotherFI>=0)
			{
				nCollected+=appendUnique(group1,otherFI);
				nCollected+=appendUnique(group1,anotherFI);
			}

			 
            if(isMyEdge && handlemultipliConnectedEdges)
            {


                SortableList<scalar> closeNess(myEdgeFaces.size(), selectMin ? 1000.0: -1000.0 );
                vector masterNormal=faces[connectingFace].normal(points);
                vector Ce=0.5*(points[meshPoints[edges[eI][0]]]+points[meshPoints[edges[eI][1]]]);
                vector tmf=faces[connectingFace].centre(points)-Ce;
                tmf/=mag(tmf)+1.0e-15;

                forAll(myEdgeFaces, fI) if ( myEdgeFaces[fI]!=connectingFace && fMarks[myEdgeFaces[fI]]==fMarks[connectingFace] )
                {
					vector tf=faces[myEdgeFaces[fI]].centre(points)-Ce;
					tf/=mag(tf)+1.0e-15;
					scalar sin=tf&masterNormal;
					scalar cos=tf&tmf;
					const double PI=3.14159265;
					double angle=std::atan2 (sin,cos) * 180 / PI;

					if ( angle<0.0) angle=360+angle;
					closeNess[fI]=angle;
                }

                if (!selectMin)        closeNess=-closeNess;

                label nei=getMinPos(closeNess);
                if (appendUnique(group1,myEdgeFaces[nei])==1)   nCollected++;

            }
        }

    }
  }
  return nCollected;
}


//=============================================================================================
//=============================================================================================
//=============================================================================================
//=============================================================================================


void correct( faceList & faces, labelList& fMarks, DynamicField<point> & points, bool handlemultipliConnectedEdges )
{ 
    Info<<"	"<<points.size()<<" points,  "<<faces.size()<<" faces, correcting:  * ";  cout.flush();

    // new points and changed faces
    DynamicList<point> addedPoints(points.size()/100+1);
    DynamicList<face>  modifiedFaces(faces.size()/100+1);
    DynamicList<label>  changedFIndices(faces.size()/100+1);

    label nProblemPoints = 0;


	Xfer<List<face> > facesFer(faces,false);
	//~ Field<point> & pointsSF=points;
	Xfer<Field<point> > pointsFer(points,false);
	MeshedSurface<face> surf1(pointsFer, facesFer);

	const labelListList& pFaces = surf1.pointFaces();
	const labelList& meshPoints = surf1.meshPoints();

	const List<face>& Sfaces = surf1.faces();



	label iLastPoint=points.size()-1;
	forAll(meshPoints, meshPI)
	{
		label pI=(meshPoints[meshPI]);
		const labelList& myFaces = pFaces[meshPI];
		if (myFaces.size()>5)
		{

			DynamicList<label> group1(myFaces.size());

			group1.append(myFaces[0]); 
			while (appendSinglyConnectedNeis(meshPI ,group1,surf1, fMarks,handlemultipliConnectedEdges));
			appendSinglyConnectedNeis(meshPI ,group1,surf1, fMarks, handlemultipliConnectedEdges);

			if(group1.size()<myFaces.size())
			{
				if( (group1.size()<myFaces.size()-2) &&  (group1.size()>2))
				{   
					
				  bool PreviouslyModified=false;
				  forAll(group1,gfI)   if (findPos(changedFIndices,group1[gfI])>=0)	PreviouslyModified=true;
				  if (!PreviouslyModified)
				  {		nProblemPoints++;

					addedPoints.append(points[pI]); ++iLastPoint;  ///. duplicate the point, it will go to the end

					forAll(group1,gfI)
					{
						face modifiedFace=Sfaces[group1[gfI]];        ///. get the face

						label iModFace=findPos(changedFIndices,group1[gfI]);
						if (iModFace>=0)	{modifiedFace=modifiedFaces[iModFace];Info<<iModFace<<"Error in correcting faces : dbl cor face"<<endl;}; 

						changedFIndices.append(group1[gfI]);
						label index=findPos(Sfaces[group1[gfI]],pI);
						if (index>=0)	modifiedFace[index]=iLastPoint; ///. change the face
						else	Info<<gfI<<":Error in correcting faces : negative array index "<<index<<"  "
							<<Sfaces[group1[gfI]]<<" "<<nProblemPoints<<" "<<group1[gfI]<<" "<<points[pI]<<endl; 

						modifiedFaces.append( modifiedFace );  ///. save the changed face
					}
				  }
				}
				else  Info<<"Point"<<pI<<", collected " <<  group1.size()<<" faces out of "<<myFaces.size()<<", skipped, as this will cause singly connected edges"<<endl; 
			}

		}
		//~ else if(myFaces.size()<3)  Info<<pI<<": wrong point : "<<points[pI]<<endl; 


	}



    Info<< nProblemPoints<< " shared edges,  "; 
    Info<<"addedPoints: "<<addedPoints.size() <<"  changedFIndices: "<<changedFIndices.size() <<"  changedFaces: "<<modifiedFaces.size()<<"   ";       

	points.append(addedPoints);
	forAll(changedFIndices,i)	faces[ changedFIndices[i] ]= modifiedFaces[i];

    Info<<faces.size()<<" faces "<<points.size()<<" points "<<endl;  


}













void correctbioti( faceList & faces, labelList& fMarks, DynamicField<point> & points, int stage )
{ 
    Info<<points.size()<<" points,  "<<faces.size()<<" faces,  stage:"<<stage<<"   correcting: ";  cout.flush();

    DynamicList<label>  deletedFacesIndices(faces.size()/100+1);

    label nProblemPoints = 0;
    label nbads = 0;


	faceList & Sfaces=faces;
	labelList pMarks(points.size(),0);
	List<labelList> pFaces(points.size());
	forAll(Sfaces, fI)
	{	const face& f = Sfaces[fI];
		forAll(f,i) ++pMarks[f[i]];
	}
	forAll(pFaces, pI)	pFaces[pI].resize(pMarks[pI]);
	pMarks=-1;
	forAll(Sfaces, fI)
	{	const face& f = Sfaces[fI];
		forAll(f,i) pFaces[f[i]][++pMarks[f[i]]]=fI;
	}

	labelList pointPointmap(points.size(),-1);




	forAll(pFaces, pI)
	{
		label pII = pI;///surf1.meshPoints()[pI];
		label n3fs(0),delFI(-1);
		const labelList& myFaces = pFaces[pI];
		if (myFaces.size()>4)
		{
			label myMark(fMarks[myFaces[0]]); bool CLine(false);
			forAll(myFaces,i)
			{
				const face& f=Sfaces[myFaces[i]];
				label mePIinf=f.which(pII);
				if (mePIinf<0) 
				{
					++nbads;//Info<<"  bad:"<<pI<<" "<<pII<<"  ";
					
				}
				else
				if (pFaces[f.nextLabel(mePIinf)].size() ==3 && pFaces[f.prevLabel(mePIinf)].size() ==3) {++n3fs; delFI=myFaces[i];}
				if(myMark!=fMarks[myFaces[i]]) CLine=true;
			}
			if (CLine && n3fs==0 && stage==2)
			{
			  forAll(myFaces,i)
			  {
				const face& f=Sfaces[myFaces[i]];
				label mePIinf=f.which(pII);
				if (mePIinf<0) 
				{
					++nbads;//Info<<"  bad:"<<pI<<" "<<pII<<"  ";
				}
				else 
				{
					label opospI=f[(mePIinf+2)%f.size()];
					if (pFaces[opospI].size() ==3 && pFaces[f.prevLabel(mePIinf)].size()>6 && pFaces[f.nextLabel(mePIinf)].size()>6)
					{
						++n3fs; delFI=myFaces[i]; pII=f.prevLabel(mePIinf);
						//~ Info<<"!"<<(mePIinf+2)%f.size()<<" "<<delFI<<"    "<<pI<<" "<<opospI<<" "<<pII<<" "<<endl;
						//~ exit(-1);
					}
				}
			  }
			}
		}

		if ( (n3fs==1 ||  (stage>=1 && n3fs>=1)) && (findPos(deletedFacesIndices,delFI)<0)  && pointPointmap[pII]==-1  && pointPointmap[pI]==-1 )
		{
			nProblemPoints++;

			const face& f=Sfaces[delFI];
			label mePIinf=f.which(pII);
			if (mePIinf<0) 
			{
				Info<<"  bad:"<<pI<<" "<<pII<<"  ";
			}
			else
			{
				int keptP,delP;
				if(f.nextLabel(mePIinf)<f.prevLabel(mePIinf))
					{keptP=f.nextLabel(mePIinf);  delP=f.prevLabel(mePIinf);}
				else{delP=f.nextLabel(mePIinf);  keptP=f.prevLabel(mePIinf);}
				
				if (pointPointmap[delP]==-1 && pointPointmap[keptP]==-1)
				{
					pointPointmap[delP]=keptP;
					deletedFacesIndices.append(delFI);
					//~ mergedPointIndices.append(keptP);
					//~ mergedPointIndices.append(delP);
				}
			}
		}
		//~ else if(myFaces.size()<3)  Info<<pI<<": wrong point : "<<points[pI]<<endl; 


	}



    Info<< nProblemPoints<< " dbl 3-nei points in face. bads:"<<nbads<<" *  "; 


	{
		label iLastPoint=-1;
		forAll(points,i)
		{
			if (pointPointmap[i]<0) 
			{
				points[++iLastPoint]=points[i];
				pointPointmap[i]=iLastPoint;
			}
			else 
			{
				if (pointPointmap[i]>=i) Info<<" Errorsddsfdf "<<endl;
				pointPointmap[i]=pointPointmap[pointPointmap[i]];
				points[pointPointmap[i]]=0.5*(points[i]+points[pointPointmap[i]]);
			}
		}
		points.resize(iLastPoint+1);
	}
	Info<<"    "<<points.size()<<" points "<<" ";  
	{
		
		forAll(deletedFacesIndices,i)	faces[ deletedFacesIndices[i] ].resize(0);
		label iLastFace=-1;
		forAll(Sfaces,i)
		{
			if(faces[i].size())
			{
				face f=faces[i];
				forAll(f,ii)  f[ii]=pointPointmap[f[ii]];
				faces[++iLastFace]=f;
				fMarks[iLastFace]=fMarks[i];
			}
		}
		faces.resize(iLastFace+1);
		fMarks.resize(iLastFace+1);
	}
    Info<<faces.size()<<" faces "<<endl; 


}





void correctbioti2( faceList & faces, labelList& fMarks, DynamicField<point> & points, int stage )
{ 
    Info<<points.size()<<" points,  "<<faces.size()<<" faces,  stage:"<< stage <<"   correcting: ";  cout.flush();


    label nProblemPoints = 0;
    label nbads = 0;
	faceList & Sfaces=faces;
	labelList pMarks(points.size(),0);
	List<labelList> pFaces(points.size());
	forAll(Sfaces, fI)
	{	const face& f = Sfaces[fI];
		forAll(f,i) ++pMarks[f[i]];
	}
	forAll(pFaces, pI)	pFaces[pI].resize(pMarks[pI]);
	pMarks=-1;
	forAll(Sfaces, fI)
	{	const face& f = Sfaces[fI];
		forAll(f,i) pFaces[f[i]][++pMarks[f[i]]]=fI;
	}


	forAll(pFaces, pI)
	{
		const labelList& myFaces = pFaces[pI];
		//~ label mySize=myFaces.size();
		/* if(mySize<4)
		//~ { forAll(myFaces,i)
		  //~ {
			//~ const face& f=Sfaces[myFaces[i]];
			//~ label mePIinf=f.which(pI);
			//~ if (mePIinf<0) 	{	++nbads;	}
			//~ else
			//~ {
				//~ if(pMarks[f.nextLabel(mePIinf)]<=0) pMarks[f.nextLabel(mePIinf)]-=1;;
				//~ if(pMarks[f.prevLabel(mePIinf)]<=0) pMarks[f.prevLabel(mePIinf)]-=1;;
			//~ }
		} }else */
		//~ if (mySize>4)
		{
			label myMark(fMarks[myFaces[0]]);
			forAll(myFaces,i)		if(myMark!=fMarks[myFaces[i]]) myMark=4;
			//~ pMarks[mshps[pI]]=myMark;
			pMarks[pI]=myMark;
		}

	}

	forAll(Sfaces, fI)
	{
		label nCLPs(0),nonCLI(-1);
		const face& f = Sfaces[fI];
		forAll(f,i)
		{
			if (pMarks[f[i]]==4) ++nCLPs;
			else if (pMarks[f[i]]==-1000) nCLPs-=1000;
			else nonCLI=i;
		}
		
		if (nCLPs==3)
		{
			label myMark(fMarks[fI]);
			label clP1(f.prevLabel(nonCLI)),clP2(f.nextLabel(nonCLI));
			label nSameMark1(0),nSameMark2(0);
			const labelList& faces1 = pFaces[f.prevLabel(nonCLI)];
			forAll(faces1,i) 	if(myMark==fMarks[faces1[i]]) ++nSameMark1;
			const labelList& faces2 = pFaces[f.nextLabel(nonCLI)];
			forAll(faces2,i) 	if(myMark==fMarks[faces2[i]]) ++nSameMark2;
			

			if( ((nSameMark1>2) != (nSameMark2>2)) || (stage && (nSameMark1>2 || nSameMark2>2)) || (stage>1 && pFaces[f[nonCLI]].size()>3))
			{
			  if((nSameMark1>2 && nSameMark1>=nSameMark2) || (stage>1 && nSameMark1>nSameMark2))
			  {
				const labelList& myFaces = pFaces[f[nonCLI]];
				label If1(-1);
				forAll(myFaces,i)  if(myFaces[i]!=fI && Sfaces[myFaces[i]].which(clP1)>=0) { If1=i; break;}
				const face& fnei = Sfaces[myFaces[If1]];
				if (If1<0) 	{++nbads; Info<<" dsdked11 "<<If1<<"  "<<f<<"  "<<endl;	continue;	}
				if (f.which(clP1)<0) {++nbads; Info<<" dsdkedsfgf "<<endl; continue;;}
				if(nSameMark1>2 || pFaces[fnei[(fnei.which(clP1)+2)%fnei.size()]].size()<4)
				{
					faces[myFaces[If1]][ fnei.which(f[nonCLI]) ]=f[(nonCLI+2)%f.size()]; 
					faces[fI][    f.which(clP1)      ]=   fnei[(fnei.which(clP1)+2)%fnei.size() ] ; 
					++nProblemPoints;
				}
			  }
			  else 
			  if(nSameMark2>2 || stage>1 )
			  {
				const labelList& myFaces = pFaces[f[nonCLI]];
				label If2(-2);
				forAll(myFaces,i)  if(myFaces[i]!=fI && Sfaces[myFaces[i]].which(clP2)>=0) { If2=i; break;}
				const face& fnei = Sfaces[myFaces[If2]];
				if (fnei.which(f[nonCLI])<0) {++nbads; Info<<" dsdked45 "<<endl; continue;;}
				if (f.which(clP2)<0) {++nbads; Info<<" dsdkedsfgf "<<endl; continue;;}
				
				//~ if (magSqr(points[f[nonCLI]]-points[clP2])>0.255*
				if(nSameMark2>2 || pFaces[fnei[(fnei.which(clP2)+2)%fnei.size()]].size()<4)
				{
					faces[myFaces[If2]][ fnei.which(f[nonCLI]) ]=f[(nonCLI+2)%f.size()]; 
					faces[fI][ f.which(clP2) ]=   fnei[(fnei.which(clP2)+2)%fnei.size()] ; 
					++nProblemPoints;
				}
			  }
			  else Info<<" Errorsshjh";
			}
				
			forAll(f,i) pMarks[f[i]]=-1000;


		}




	}



    Info<< nProblemPoints<< " flips. bads:"<<nbads<<" *       "; 



    Info<<faces.size()<<" faces, "<<points.size()<<"  points"<<endl;; 


}



