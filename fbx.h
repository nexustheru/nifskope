#define FbxSDK_NEW_API
#include "../nifmodel.h"

#include "../NvTriStrip/qtwrapper.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QApplication>
//Fbx
#include <Fbxsdk.h>

#include "../options.h"

#define tr(x) QApplication::tr(x)
using namespace fbxsdk_2013_3;
using namespace std;
class fbxXfile 
{
public:

         fbxXfile(char*);
         void exportFbx( const NifModel * nif, const QModelIndex & index );
         void traverseFbxNodes(FbxNode* node) ;
		 void importFbx( NifModel * nif, const QModelIndex & index );
        ~fbxXfile();

private:

          char Name[25];
          float *normals;
          int numNormals;

          int *indices;
          int numIndices;

          int numVertices;
//STRUCTS
struct vertex
{
float x,y,z;
};

struct texturecoords
{
float a,b;
};

struct poligon
{
int a,b,c;
};

};
