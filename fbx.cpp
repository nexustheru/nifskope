#define FBXSDK_NEW_API
#define MAX_VERTICES 80000
#include "../nifmodel.h"

#include "../NvTriStrip/qtwrapper.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QApplication>
#include <QDomDocument>
#include "../gl/gltex.h"

#include "../options.h"
#include <fbxsdk.h>
#define tr(x) QApplication::tr(x)
using namespace fbxsdk_2013_3;
using namespace std;

static void addLink( NifModel * nif, QModelIndex iBlock, QString name, qint32 link )
{
	QModelIndex iArray = nif->getIndex( iBlock, name );
	QModelIndex iSize = nif->getIndex( iBlock, QString( "Num %1" ).arg( name ) );
	int numIndices = nif->get<int>( iSize );
	nif->set<int>( iSize, numIndices + 1 );
	nif->updateArray( iArray );
	nif->setLink( iArray.child( numIndices, 0 ), link );
}


struct fbxtriangle
{
	float x,y,z;
};

struct fbxuv
{
	int a,b;
};

struct  bonelist
{
	FbxDouble3 bones;
	FbxDouble3 rotation;
	FbxDouble3 scaling;
};

QVector<Vector3> dublein(FbxDouble3 duble)
{
	QVector<Vector3> pos;
	pos.append(Vector3((float)duble[0],(float)duble[1],(float)duble[2]));
	return pos;
}

void getskeleton(FbxNode* rootnode,const QModelIndex & index,NifModel * nif)
{
QPersistentModelIndex iNode, iBone, iShape, iMaterial, iData, iTexProp, iTexSource, iFbx;
QModelIndex iBlock = nif->getBlock( index );
bonelist fbones;
QDomElement* parent;
FbxNode* bonenode;
QMessageBox::information(0,"fbx","getting bones!"); 
	for(int nod=0 ; nod < rootnode->GetChildCount(true) ; ++nod)
	{
			bonenode=rootnode->GetChild(nod);
			FbxDouble3 translation = bonenode->LclTranslation.Get();
            iNode = nif->insertNiBlock( "NiNode" );
            nif->set<QString>( iNode, "Name","root");
           

	}

}

FbxMesh* get_shapes(const NifModel * nif, const QModelIndex & index,FbxManager* manager)
{
int i ;
FbxMesh* rootmesh=FbxMesh::Create(manager,"RootMesh");
QPersistentModelIndex iNode, iShape, iMaterial, iData, iTexProp, iTexSource, iFbx;
QModelIndex iBlock = nif->getBlock( index );
QVector<Vector3> verts = nif->getArray<Vector3>( iData, "Vertices" );
return rootmesh;
}

void exportFbx(const NifModel * nif, const QModelIndex & index)
{
try
{
///QT
QWidget* dialog=new QWidget();
QString fileName=QFileDialog::getSaveFileName(dialog,tr("Export Fbx"),QDir::currentPath(),tr("Fbx (*.FBX *.fbx)"));
///FBX
FbxManager *manager = FbxManager::Create();
FbxIOSettings *ioSettings = FbxIOSettings::Create(manager, IOSROOT);
manager->SetIOSettings(ioSettings);
FbxExporter *exporter=FbxExporter::Create(manager,"");
const bool result= exporter->Initialize("exbox.fbx",-1,manager->GetIOSettings());
if(!result)
{
QMessageBox::information(0,"error",exporter->GetLastErrorString());
}
else
{
int count;
FbxDocument *scene = FbxDocument::Create(manager,"tempName");
FbxDocumentInfo* lDocInfo = FbxDocumentInfo::Create(manager,"DocInfo");
lDocInfo->mTitle = "Fbx Niftools";
lDocInfo->mSubject = "Illustrates the creation of FbxDocument with geometries, materials and lights from Nifskope.";
lDocInfo->mAuthor = "Nifskope.exe sample program.";
lDocInfo->mRevision = "rev. 1.0";
lDocInfo->mKeywords = "Fbx Nifskope";
lDocInfo->mComment = "no particular comments required.";
QMessageBox::information(0,"documentation created","fbx settings set");
    // add the documentInfo
scene->SetDocumentInfo(lDocInfo);
//nifskopemesh goes here
//FbxNode* rootnode=FbxNode::Create(manager,"RootNode");
//
scene->AddRootMember(get_shapes(nif,index,manager));
QMessageBox::information(0,"nodes/meshes","shapes added");
count = scene->GetRootMemberCount();  // lCount = 1: only the lPlane
count = scene->GetMemberCount();
const bool res=exporter->Export(scene);
if(!res)
{
QMessageBox::information(0,"error! U need to open nifskope with admin rights then export again",exporter->GetLastErrorString());
}
else
{
QMessageBox::information(0,"Export Fbx","File has been exported correcly");
}
exporter->Destroy();
}	
}
catch(FbxError* ex)
{
	QMessageBox::information(0,"error",ex->GetLastErrorString());
}
}

QVector<Vector2> LoadUVInformation(FbxMesh* pMesh)
{
    //get all UV set names
    FbxStringList lUVSetNameList;
    pMesh->GetUVSetNames(lUVSetNameList);
	QVector<Vector2> uvset1;
    //iterating over all uv sets
    for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
    {
        //get lUVSetIndex-th uv set
        const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
        const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

        if(!lUVElement)
            continue;

        //index array, where holds the index referenced to the uv data
        const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
        const int lIndexCount= (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

        //iterating through the data by polygon
        const int lPolyCount = pMesh->GetPolygonCount();

        if( lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint )
        {
            for( int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex )
            {
                // build the max index array that we need to pass into MakePoly
                const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
                for( int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex )
                {
                    FbxVector2 lUVValue;

                    //get the index of the current vertex in control points array
                    int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex,lVertIndex);

                    //the UV index depends on the reference mode
                    int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

                    lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);
					uvset1.append(Vector2(lUVValue.mData[0],lUVValue.mData[1]));
                        //User TODO:
                        //Print out the value of UV(lUVValue) or log it to a file
                        return uvset1;
                    //User TODO:
                    //Print out the value of UV(lUVValue) or log it to a file
                }
            }
        }
        else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
        {
            int lPolyIndexCounter = 0;
            for( int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex )
            {
                // build the max index array that we need to pass into MakePoly
                const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
                for( int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex )
                {
                    if (lPolyIndexCounter < lIndexCount)
                    {
                        FbxVector2 lUVValue;

                        //the UV index depends on the reference mode
                        int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

                        lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);
						
                        uvset1.append(Vector2(lUVValue.mData[0],lUVValue.mData[1]));
                        //User TODO:
                        //Print out the value of UV(lUVValue) or log it to a file
                        return uvset1;
                        lPolyIndexCounter++;
                    }
                }
            }
        }
    }
	
}

const char* qconvertname(QString fname)
 {
    QString temp=fname.replace("//","\\\\");
    return temp.toUtf8().constData();
}

void importFbx( NifModel * nif, const QModelIndex & index )
{  
QPersistentModelIndex iNode, iBone, iShape, iMaterial, iData, iTexProp, iTexSource, iFbx;
QModelIndex iBlock = nif->getBlock( index );
///QT
QWidget* dialog=new QWidget();
QString fileName=QFileDialog::getOpenFileName(dialog,tr("Import Fbx"),QDir::currentPath(),tr("Fbx (*.FBX *.fbx)"));

///FBX
FbxManager *manager = FbxManager::Create();
FbxIOSettings *ioSettings = FbxIOSettings::Create(manager, IOSROOT);

ioSettings->SetBoolProp(EXP_FBX_MATERIAL,        true);
ioSettings->SetBoolProp(EXP_FBX_TEXTURE,         true);
ioSettings->SetBoolProp(EXP_FBX_EMBEDDED,        false);
ioSettings->SetBoolProp(EXP_FBX_SHAPE,           true);
ioSettings->SetBoolProp(EXP_FBX_GOBO,            true);
ioSettings->SetBoolProp(EXP_FBX_ANIMATION,       true);
ioSettings->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
manager->SetIOSettings(ioSettings);
FbxImporter *importer=FbxImporter::Create(manager,"");
const bool result= importer->Initialize("box.fbx",-1,manager->GetIOSettings());
if(!result)
{
QMessageBox::information(0,"error",importer->GetLastErrorString());
}
else
{
FbxScene *scene = FbxScene::Create(manager,"tempName");
importer->Import(scene);
importer->Destroy();
FbxNode* rootNode=scene->GetRootNode();
QMessageBox::information(0,"fbx","import fbx ok!"); 
FbxMesh* mesh = NULL;
FbxGeometryConverter convert(manager);

try
{

for (int childi = 0; childi < rootNode->GetChildCount(true); ++ childi)
	{
		
		if(mesh=rootNode->GetChild(childi)->GetMesh())
		{
			QMessageBox::information(0,"fbx","MESH FOUND!");
			break;
		}

		if(mesh->IsTriangleMesh()==false)
		{
            QMessageBox::information(0,"optimizing","TRIANGLUALTING MESH!");
			convert.TriangulateInPlace(scene->GetRootNode()->GetChild(0));
			break;
		}
		mesh=rootNode->GetChild(childi)->GetMesh();
	}
        
        ///GLOBALS
        FbxVector4* controlPoints = mesh->GetControlPoints();
		int ncount =mesh->GetElementNormalCount();
		int numVerts=mesh->GetControlPointsCount();
		QVector<Vector2> uvset;
		FbxVector4 themesh;
		QVector<Vector3> positions;
		QVector<Triangle> triangle;
		int polcount=mesh->GetPolygonCount();
		QVector<Vector3> normalsv;
		QVector<Vector3> normalbi;
        QVector<Vector3> smooth;
		


		///VERTEX

        FbxGeometryElementUV* uvi=mesh->GetElementUV();
        FbxGeometryElementNormal* getnorm=mesh->GetElementNormal();
        FbxGeometryElementBinormal* binorm=mesh->GetElementBinormal();
		for( int vert = 0; vert < numVerts; vert++ )
		    {
			themesh=mesh->GetControlPointAt(vert);
			int x=themesh.mData[0];
			int y=themesh.mData[1];
			int z=themesh.mData[2];
            positions.append(Vector3(x,y,z));

			 //uv

			 uvset.append(Vector2(uvi->GetDirectArray().GetAt(vert)[0],uvi->GetDirectArray().GetAt(vert)[1]));
         
		     //normal
		     
			 normalsv.append(Vector3(getnorm->GetDirectArray().GetAt(vert)[0],getnorm->GetDirectArray().GetAt(vert)[1],getnorm->GetDirectArray().GetAt(vert)[2]));
		
		     //binormal
			  
			 normalbi.append(Vector3(binorm->GetDirectArray().GetAt(vert)[0],binorm->GetDirectArray().GetAt(vert)[1],binorm->GetDirectArray().GetAt(vert)[2]));

			 //smoothing
		     
		   }



		     ///POLYGON  issues with custom mesh
		   for (int polyIndex = 0; polyIndex < polcount; ++polyIndex)
              {
			 int size=mesh->GetPolygonSize(polyIndex);
             fbxtriangle vertex;
             vertex.x = (float)mesh->GetPolygonVertex(polyIndex,0);
             vertex.y = (float)mesh->GetPolygonVertex(polyIndex,1);
             vertex.z = (float)mesh->GetPolygonVertex(polyIndex,2);
			 triangle.append(Triangle(vertex.x,vertex.y,vertex.z));
         
		      }
		
            
		   ///bones exprimental
         
		   getskeleton(rootNode,index,nif);

		    ///NIFSKOPE NODES
		    iNode = nif->insertNiBlock( "NiNode" );
            nif->set<QString>( iNode, "Name", "Scene Root" );
		    iData = nif->insertNiBlock( "NiTriShapeData" );
		    nif->set<int>( iData, "Num Vertices",numVerts);
		    nif->set<int>( iData, "Has Vertices", 1 );
		    nif->updateArray( iData, "Vertices" );
		    nif->setArray<Vector3>( iData, "Vertices",positions);
		    iShape = nif->insertNiBlock( "NiTriShape" );
		    nif->set<QString>( iShape, "Name", QString::fromUtf8(mesh->GetName()) );
		    nif->set<int>( iData, "Has Normals", 1 );
			nif->updateArray( iData, "Normals" );
			nif->setArray<Vector3>( iData, "Normals", normalsv);
			nif->set<int>( iData, "Has UV", 1 );
			int cNumUVSets = nif->get<int>( iData, "Num UV Sets");// keep things the way they are
			nif->set<int>( iData, "Num UV Sets", 4097 | cNumUVSets );// keep things the way they are
			nif->set<int>( iData, "Num UV Sets 2", 1 | cNumUVSets );// keep things the way they are
			QModelIndex iTexCo = nif->getIndex( iData, "UV Sets" );
			if ( ! iTexCo.isValid() )
				iTexCo = nif->getIndex( iData, "UV Sets 2" );
			nif->updateArray( iTexCo );
			nif->updateArray( iTexCo.child( 0, 0 ) );
			nif->setArray<Vector2>( iTexCo.child( 0, 0 ), uvset);		
            nif->set<int>( iData, "Has Triangles", 1 );
			nif->set<int>( iData, "Num Triangles", polcount  );
			nif->set<int>( iData, "Num Triangle Points",polcount * 3 );
			nif->updateArray( iData, "Triangles" );
			nif->setArray<Triangle>( iData, "Triangles", triangle);	
		    nif->setLink( iShape, "Data", nif->getBlockNumber( iData ) );
		    addLink( nif, iNode, "Children", nif->getBlockNumber( iShape ) );
			iMaterial = nif->insertNiBlock( "BSLightingShaderProperty" );
			nif->set<Color3>( iMaterial, "Ambient Color", Color3( 0, 0, 0 ) );
			nif->set<Color3>( iMaterial, "Diffuse Color", Color3( 0, 0, 0 ) );
			nif->set<Color3>( iMaterial, "Specular Color", Color3( 0, 0, 0 ) );
			nif->set<Color3>( iMaterial, "Emissive Color", Color3( 0, 0, 0 ) );
			nif->updateArray( iData, "Bitangents" );
			nif->setArray<Vector3>( iData, "Bitangents", normalbi);
			iTexSource = nif->insertNiBlock( "BSShaderTextureSet" );
			nif->setLink( iMaterial, "Texture Set",nif->getBlockNumber( iTexSource ) );
            addLink( nif, iShape, "Properties", nif->getBlockNumber( iMaterial ) );
   
}

catch(FbxError* thiserror )
{
	QMessageBox::information(0,"error",thiserror->GetLastErrorString());
}
}
}