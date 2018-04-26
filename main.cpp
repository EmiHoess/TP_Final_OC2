#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"
#include "Map.h"

#include <iostream>
#include <fstream>

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 


#define MESHFILENAME L"misc\\ball.x"
#define TXFILE_FLOOR L"floor.jpg"
#define FLOOR_TILECOUNT 2


struct SStripData
{
    LPDIRECT3DINDEXBUFFER9 m_pStrips;          // strip indices (single strip)
    LPDIRECT3DINDEXBUFFER9 m_pStripsMany;      // strip indices (many strips)

    DWORD m_cStripIndices;
    DWORD* m_rgcStripLengths;
    DWORD m_cStrips;

    SStripData() : m_pStrips( NULL ),
                   m_pStripsMany( NULL ),
                   m_cStripIndices( 0 ),
                   m_rgcStripLengths( NULL )
    {
    }
};


struct SMeshData
{
    LPD3DXMESH m_pMeshSysMem;      // System memory copy of mesh

    LPD3DXMESH m_pMesh;            // Local version of mesh, copied on resize
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // vertex buffer of mesh

    SStripData* m_rgStripData;      // strip indices split by attribute
    DWORD m_cStripDatas;

            SMeshData() : m_pMeshSysMem( NULL ),
                          m_pMesh( NULL ),
                          m_pVertexBuffer( NULL ),
                          m_rgStripData( NULL ),
                          m_cStripDatas( 0 )
            {
            }

    void    ReleaseLocalMeshes()
    {
        SAFE_RELEASE( m_pMesh );
        SAFE_RELEASE( m_pVertexBuffer );
    }

    void    ReleaseAll()
    {
        SAFE_RELEASE( m_pMeshSysMem );
        SAFE_RELEASE( m_pMesh );
        SAFE_RELEASE( m_pVertexBuffer );

        for( DWORD iStripData = 0; iStripData < m_cStripDatas; iStripData++ )
        {
            SAFE_RELEASE( m_rgStripData[iStripData].m_pStrips );
            SAFE_RELEASE( m_rgStripData[iStripData].m_pStripsMany );
            delete[] m_rgStripData[iStripData].m_rgcStripLengths;
        }

        delete[] m_rgStripData;
        m_rgStripData = NULL;
        m_cStripDatas = 0;
    }
};


ID3DXMesh*                  g_pMeshFloor = NULL;    // floor geometry
D3DXMATRIXA16               g_mxFloor;              // floor world xform
D3DMATERIAL9                g_MatFloor;             // floor material
IDirect3DTexture9*          g_pTxFloor = NULL;      // floor texture


double						tiempo;
int							minutos = 0;
int							segundos;
int							points = 0;
bool						gameOver = false;
int							level = 1;
bool						victory = false;
int							counterVicSec = -1;
int							tiempoInicial = 0;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

CDXUTDialogResourceManager  g_DialogResourceManager;	// manager for shared resources of dialogs
	
CD3DSettingsDlg             g_SettingsDlg;				// Device settings dialog

CDXUTDialog                 g_HUD;                  // dialog for standard controls
	
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
	
CFirstPersonCamera          g_Camera;

bool						lastMButtonState = false;
	
	




// Aca vamos a guardar las variables referentes al device


ID3DXFont*                  g_pFont = NULL;         // Font for drawing text
ID3DXSprite*                g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*                g_pEffect = NULL;       // D3DX effect interface
               // A model viewing camera


IDirect3DTexture9*          g_pDefaultTex = NULL;   // La textura predeterminada del Device, usada para las Meshes cargadas...


bool                        g_bShowHelp = true;     // If true, it renders the UI control text


bool                        g_bShowVertexCacheOptimized = true;
bool                        g_bShowStripReordered = false;
bool                        g_bShowStrips = false;
bool                        g_bShowSingleStrip = false;


bool						leftButtonPressed;

// Flag para indicar si estoy trabajando con vertices de 32bits
bool                        g_bForce32ByteFVF = true;
bool                        g_bCantDoSingleStrip = false;// Single strip would be too many primitives
D3DXVECTOR3                 g_vObjectCenter;        // Center of bounding sphere of object
FLOAT                       g_fObjectRadius;        // Radius of bounding sphere of object
D3DXMATRIXA16               g_matWorld;
int                         g_cObjectsPerSide = 1;  // sqrt of the number of objects to draw
DWORD                       g_dwMemoryOptions = D3DXMESH_MANAGED;

// Varios Buffers para la optimización del Modelo de la pelotita
SMeshData                   g_MeshAttrSorted;
SMeshData                   g_MeshStripReordered;
SMeshData                   g_MeshVertexCacheOptimized;

// Cantidad de Materiales del Modelo de las pelotitas
DWORD                       g_dwNumMaterials = 0;
IDirect3DTexture9**         g_ppMeshTextures = NULL;
// Lista de Materiales para el Modelo de las pelotitas
D3DMATERIAL9*               g_pMeshMaterials = NULL;

LPDIRECT3DVERTEXBUFFER9		g_pVB = NULL;

bool						g_explosion = false;


// Physics variables
float lastTime;
Map bolas(g_cObjectsPerSide);

std::ofstream out;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_MESHTYPE            5
#define IDC_GRIDSIZE            6
#define IDC_PRIMTYPE            7


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
                                  void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext);
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );

void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );


HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void RenderText(IDirect3DDevice9* pd3dDevice);
HRESULT LoadMeshData( IDirect3DDevice9* pd3dDevice, LPCWSTR wszMeshFile, LPD3DXMESH* pMeshSysMemLoaded,
                      LPD3DXBUFFER* ppAdjacencyBuffer );
HRESULT OptimizeMeshData( LPD3DXMESH pMeshSysMem, LPD3DXBUFFER pAdjacencyBuffer, DWORD dwOptFlags,
                          SMeshData* pMeshData );
HRESULT UpdateLocalMeshes( IDirect3DDevice9* pd3dDevice, SMeshData* pMeshData );
HRESULT DrawMeshData( ID3DXEffect* pEffect, SMeshData* pMeshData );
HRESULT LoadMeshDataOptimized(IDirect3DDevice9* pd3dDevice, wchar_t* str);


//--------------------------------------------------------------------------------------
// Handles the GUI events
// Modificar para nuestro ejemplo
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
			g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
        case IDC_MESHTYPE:
            switch( ( size_t )( ( CDXUTComboBox* )pControl )->GetSelectedData() )
            {
                case 0:
                    g_bShowVertexCacheOptimized = true;
                    g_bShowStripReordered = false;
                    break;
                case 1:
                    g_bShowVertexCacheOptimized = false;
                    g_bShowStripReordered = true;
                    break;
                case 2:
                    g_bShowVertexCacheOptimized = false;
                    g_bShowStripReordered = false;
                    break;
            }
            break;

        case IDC_PRIMTYPE:
            switch( ( size_t )( ( CDXUTComboBox* )pControl )->GetSelectedData() )
            {
                case 0:
                    g_bShowStrips = false;
                    g_bShowSingleStrip = false;
                    break;
                case 1:
                    g_bShowStrips = false;
                    g_bShowSingleStrip = true;
                    break;
                case 2:
                    g_bShowStrips = true;
                    g_bShowSingleStrip = false;
                    break;
            }
            break;

        case IDC_GRIDSIZE:
            g_cObjectsPerSide = ( int )( size_t )( ( CDXUTComboBox* )pControl )->GetSelectedData();
            break;
    }
}


void InitApp()
{
    // Initialize dialogs
	g_SettingsDlg.Init( &g_DialogResourceManager);
	g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );
	
	// Esto no va mas
    InitApp();

    DXUTInit( true, true ); 
    DXUTSetHotkeyHandling( true, true, true );  
    DXUTCreateWindow( L"TP Final Orga 2" );
    DXUTCreateDevice( true, 640, 480 );

    DXUTMainLoop();


    return DXUTGetExitCode();
}






//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}

//------------------------
// Load Mesh Data function
//------------------------

HRESULT LoadMeshData( IDirect3DDevice9* pd3dDevice, LPCWSTR wszMeshFile, LPD3DXMESH* pMeshSysMemLoaded,
                      LPD3DXBUFFER* ppAdjacencyBuffer )
{
	// Vertex Buffer del Modelo
    LPDIRECT3DVERTEXBUFFER9 pMeshVB = NULL;
    // Buffer de Materiales del Modelo
	LPD3DXBUFFER pD3DXMtrlBuffer = NULL;
	
	// Para recorrer los vertices
    void* pVertices;
    WCHAR strMesh[512];
    HRESULT hr = S_OK;

	// Para el Vertex Buffer del Modelo 3D
    LPD3DXMESH pMeshSysMem = NULL;
    LPD3DXMESH pMeshTemp;
    D3DXMATERIAL* d3dxMaterials;

    // Busco la dirección donde esta el Model 3D
    if( FAILED( hr = DXUTFindDXSDKMediaFileCch( strMesh, 512, wszMeshFile ) ) )
        goto End;

    // Cargo el Model 3D desde el archivo .x y lo guardo en pD3DXMtrlBuffer
    hr = D3DXLoadMeshFromX( strMesh, D3DXMESH_SYSTEMMEM, pd3dDevice,
                            ppAdjacencyBuffer, &pD3DXMtrlBuffer, NULL,
                            &g_dwNumMaterials, &pMeshSysMem );
    // Si hubo error, termino
	if( FAILED( hr ) )
        goto End;

    // Apunto al arreglo de Materiales del Model que acabo de cargar
    d3dxMaterials = ( D3DXMATERIAL* )pD3DXMtrlBuffer->GetBufferPointer();
    
	// Genero espacio suficiente para hacer el arreglo de materiales
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    if( g_pMeshMaterials == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }
	
	// Reservo espacio suficiente para hacer el arreglo de texturas
    g_ppMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
    if( g_ppMeshTextures == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    
	// Lleno el arreglo de Materiales del que habia reservado memoria anteriormente
	// y defino su textura, en caso de tenerla
	for( DWORD i = 0; i < g_dwNumMaterials; i++ )
    {
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;
        
		g_ppMeshTextures[i] = NULL;

        
        WCHAR strPath[512];
        if( d3dxMaterials[i].pTextureFilename != NULL )
        {
			// Busco el lugar donde esta guardada la textura del material del Modelo...
            WCHAR wszBuf[MAX_PATH];
            MultiByteToWideChar( CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, wszBuf, MAX_PATH );
            wszBuf[MAX_PATH - 1] = L'\0';
            DXUTFindDXSDKMediaFileCch( strPath, 512, wszBuf );

            // Cargo la textura
            D3DXCreateTextureFromFile( pd3dDevice, strPath, &g_ppMeshTextures[i] );
        }
        else
        {
            // Uso la textura default del Device (color gris)
            g_ppMeshTextures[i] = g_pDefaultTex;
            g_ppMeshTextures[i]->AddRef();
        }
    }

    // Termine de Cargar los Materiales y la Textura de los mismos.
    SAFE_RELEASE( pD3DXMtrlBuffer );

    // Abro el Vertex Buffer para generar una Esfera, que se usa en la camara (esto habria que sacarlo para este ejemplo).
    hr = pMeshSysMem->GetVertexBuffer( &pMeshVB );
    if( SUCCEEDED( hr ) )
    {
        hr = pMeshVB->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
        if( SUCCEEDED( hr ) )
        {
            D3DXComputeBoundingSphere( ( D3DXVECTOR3* )pVertices, pMeshSysMem->GetNumVertices(),
                                       D3DXGetFVFVertexSize( pMeshSysMem->GetFVF() ),
                                       &g_vObjectCenter, &g_fObjectRadius );
            pMeshVB->Unlock();
        }
        pMeshVB->Release();
    }
    else
        goto End;

    // Necesito las normales de los Vertices, me fijo si el Modelo ya tiene las normales hechas...
    bool bNormalsInFile = ( pMeshSysMem->GetFVF() & D3DFVF_NORMAL );

    // Si estoy usando Vertices de 32 Bits, chequeo FVF
    if( g_bForce32ByteFVF )
    {
        // Si el tipo de Vertice no es como yo quiero, lo cambio
        if( pMeshSysMem->GetFVF() != ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) )
        {
            hr = pMeshSysMem->CloneMeshFVF( pMeshSysMem->GetOptions(), D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1,
                                            pd3dDevice, &pMeshTemp );
            if( FAILED( hr ) )
                goto End;
			
			// Tiro lo que tenia antes
            pMeshSysMem->Release();
			// y ahora lo apunto con el nuevo Vertex Buffer
            pMeshSysMem = pMeshTemp;
        }
    }
        // Si no, solo me fijo que tenga Normales los vertices
    else if( !( pMeshSysMem->GetFVF() & D3DFVF_NORMAL ) )
    {
        hr = pMeshSysMem->CloneMeshFVF( pMeshSysMem->GetOptions(), pMeshSysMem->GetFVF() | D3DFVF_NORMAL,
                                        pd3dDevice, &pMeshTemp );
        if( FAILED( hr ) )
            goto End;

        // Tiro lo que tenia antes
        pMeshSysMem->Release();
		// y ahora lo apunto con el nuevo Vertex Buffer
        pMeshSysMem = pMeshTemp;
    }

    // Si no tiene normales, las calculo
    if( !bNormalsInFile )
        D3DXComputeNormals( pMeshSysMem, NULL );

    *pMeshSysMemLoaded = pMeshSysMem;
    pMeshSysMem = NULL;

End:
    SAFE_RELEASE( pMeshSysMem );

    return hr;
}



//------------------------
// Load Mesh Data function
//------------------------

HRESULT OptimizeMeshData( LPD3DXMESH pMeshSysMem, LPD3DXBUFFER pAdjacencyBuffer, DWORD dwOptFlags,
                          SMeshData* pMeshData )
{
    HRESULT hr = S_OK;
    LPD3DXBUFFER pbufTemp = NULL;

    // Attribute sort - the un-optimized mesh option
    // remember the adjacency for the vertex cache optimization
    hr = pMeshSysMem->Optimize( dwOptFlags | D3DXMESH_SYSTEMMEM,
                                ( DWORD* )pAdjacencyBuffer->GetBufferPointer(),
                                NULL, NULL, NULL, &pMeshData->m_pMeshSysMem );
    if( FAILED( hr ) )
        goto End;

	// Si la optimizacion fue lograda, lleno la clase SMeshData

	// Es una tira de datos por cada Material
    pMeshData->m_cStripDatas = g_dwNumMaterials;
    pMeshData->m_rgStripData = new SStripData[ pMeshData->m_cStripDatas ];
    if( pMeshData->m_rgStripData == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }
	
	// Si llego aca es porque se puede realizar la optimizacion para mas de una Tira de Datos
    g_bCantDoSingleStrip = false;
	// Por cada Material convierto a SingleStrip
    for( DWORD iMaterial = 0; iMaterial < g_dwNumMaterials; iMaterial++ )
    {
		// Llamo a la funcion de Direct3D, se que puedo llamarla porque pude optimizar...
        hr = D3DXConvertMeshSubsetToSingleStrip( pMeshData->m_pMeshSysMem, iMaterial,
                                                 D3DXMESH_IB_MANAGED, &pMeshData->m_rgStripData[iMaterial].m_pStrips,
                                                 &pMeshData->m_rgStripData[iMaterial].m_cStripIndices );
        if( FAILED( hr ) )
            goto End;
		

        UINT primCount = pMeshData->m_rgStripData[iMaterial].m_cStripIndices - 2;

        IDirect3DDevice9* pd3dDevice;
        D3DCAPS9 d3dCaps;
        pMeshSysMem->GetDevice( &pd3dDevice );
        pd3dDevice->GetDeviceCaps( &d3dCaps );
        SAFE_RELEASE( pd3dDevice );

        if( primCount > d3dCaps.MaxPrimitiveCount )
        {
            g_bCantDoSingleStrip = true;
        }

        hr = D3DXConvertMeshSubsetToStrips( pMeshData->m_pMeshSysMem, iMaterial,
                                            D3DXMESH_IB_MANAGED, &pMeshData->m_rgStripData[iMaterial].m_pStripsMany,
                                            NULL, &pbufTemp, &pMeshData->m_rgStripData[iMaterial].m_cStrips );
        if( FAILED( hr ) )
            goto End;

        pMeshData->m_rgStripData[iMaterial].m_rgcStripLengths = new
            DWORD[pMeshData->m_rgStripData[iMaterial].m_cStrips];
        if( pMeshData->m_rgStripData[iMaterial].m_rgcStripLengths == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto End;
        }
        memcpy( pMeshData->m_rgStripData[iMaterial].m_rgcStripLengths,
                pbufTemp->GetBufferPointer(),
                sizeof( DWORD ) * pMeshData->m_rgStripData[iMaterial].m_cStrips );
    }

End:
    SAFE_RELEASE( pbufTemp );

    return hr;
}



//------------------------
// Load Mesh Data function
//------------------------
HRESULT UpdateLocalMeshes( IDirect3DDevice9* pd3dDevice, SMeshData* pMeshData )
{
    HRESULT hr = S_OK;

    // if a mesh was loaded, update the local meshes
    if( pMeshData->m_pMeshSysMem != NULL )
    {
        hr = pMeshData->m_pMeshSysMem->CloneMeshFVF( g_dwMemoryOptions | D3DXMESH_VB_WRITEONLY,
                                                     pMeshData->m_pMeshSysMem->GetFVF(),
                                                     pd3dDevice, &pMeshData->m_pMesh );
        if( FAILED( hr ) )
            return hr;

        hr = pMeshData->m_pMesh->GetVertexBuffer( &pMeshData->m_pVertexBuffer );
        if( FAILED( hr ) )
            return hr;
    }
    return hr;
}




//------------------------
// Load Mesh Data function
//------------------------
HRESULT DrawMeshData( IDirect3DDevice9* pd3dDevice, ID3DXEffect* pEffect, SMeshData* pMeshData )
{
    HRESULT hr;
    //DWORD iCurFace;

    V( pEffect->SetTechnique( "RenderScene" ) );
    UINT cPasses;
    V( pEffect->Begin( &cPasses, 0 ) );
    for( UINT p = 0; p < cPasses; ++p )
    {
        V( pEffect->BeginPass( p ) );

        // Set and draw each of the materials in the mesh
        for( DWORD iMaterial = 0; iMaterial < g_dwNumMaterials; iMaterial++ )
        {
			V( pEffect->SetVector( "g_vDiffuse", ( D3DXVECTOR4* )&g_pMeshMaterials[iMaterial].Diffuse ) );
            V( pEffect->SetTexture( "g_txScene", g_ppMeshTextures[iMaterial] ) );
            V( pEffect->CommitChanges() );
            
            V( pMeshData->m_pMesh->DrawSubset( iMaterial ) );
        }
        V( pEffect->EndPass() );
    }
    V( pEffect->End() );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;

	// Creo un Dialog Resource Manager para el device
	V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
	// Creo un Setting Dialog para el device
	V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
	
	// Genero una textura para el device que voy a usar como Predeterminada
    V_RETURN( pd3dDevice->CreateTexture( 1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_pDefaultTex, NULL ) );
    
	// Abro la textura para cambiarle el color
	D3DLOCKED_RECT lr;
    V_RETURN( g_pDefaultTex->LockRect( 0, &lr, NULL, 0 ) );
    *( LPDWORD )lr.pBits = D3DCOLOR_RGBA( 255, 255, 255, 255 );
    V_RETURN( g_pDefaultTex->UnlockRect( 0 ) );

    // Creo un tipo de Letra que voy a usar que voy a usar en los controles...
    V_RETURN( D3DXCreateFont( pd3dDevice, 30, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              L"Arial", &g_pFont ) );

    
	
	// Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
    // shader debugger. Debugging vertex shaders requires either REF or software vertex 
    // processing, and debugging pixel shaders requires REF.  The 
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
    // shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile 
    // against the next higher available software target, which ensures that the 
    // unoptimized shaders do not exceed the shader model limitations.  Setting these 
    // flags will cause slower rendering since the shaders will be unoptimized and 
    // forced into software.  See the DirectX documentation for more information about 
    // using the shader debugger.
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DXSHADER_DEBUG;
    #endif

#ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
#ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    
		
	// Busco la posición donde tengo guardado el archivo con los "Effects" (PS y VS)
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"shaders.fx" ) );
    // Cargo el archivo de "Effects en el Device"
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags,
                                        NULL, &g_pEffect, NULL ) );

	
	LoadMeshDataOptimized(pd3dDevice, MESHFILENAME);
	
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, TXFILE_FLOOR ) );
    V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, str, &g_pTxFloor ) );


    //D3DXMatrixTranslation( &g_matWorld, -g_vObjectCenter.x, -g_vObjectCenter.y, -g_vObjectCenter.z );
	
	
	// Setup the camera with view matrix
    D3DXVECTOR3 vEye( 0, 30, 50.0f );
    D3DXVECTOR3 vAt( 0, 10, 0 );
	
    g_Camera.SetViewParams( &vEye, &vAt );
    g_Camera.SetScalers( 0.01f, 50.0f );

	g_Camera.SetResetCursorAfterMove(true);
	D3DXVECTOR3 *min, *max;

	min = new D3DXVECTOR3(-95, -95, -95);
	max = new D3DXVECTOR3(95, 95, 95);
	
	g_Camera.SetClipToBoundary(true, min, max);
	

	g_Camera.SetRotateButtons(false, false, false, true);
	g_Camera.SetEnableYAxisMovement(false);
	g_Camera.SetResetCursorAfterMove(true);
	
	D3DXMATRIX mx;
    // floor geometry transform
    
	D3DXMatrixRotationX( &g_mxFloor, -D3DX_PI / 2.0f );
    D3DXMatrixRotationY( &mx, D3DX_PI / 4.0f );
    D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
    D3DXMatrixTranslation( &mx, 0.0f, 0.0f, 0.0f );
    D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );

	

	//D3DXMatrixScaling ( &g_mxFloor, 50.5f, 50.5f, 51.5f);

    // set material for floor
    g_MatFloor.Diffuse = D3DXCOLOR( 1.f, 1.f, 1.f, .75f );
    g_MatFloor.Ambient = D3DXCOLOR( 1.f, 1.f, 1.f, 1.f );
    g_MatFloor.Specular = D3DXCOLOR( 0.f, 0.f, 0.f, 1.f );
    g_MatFloor.Emissive = D3DXCOLOR( .0f, 0.f, 0.f, 0.f );
    g_MatFloor.Power = 0.f;

	//pd3dDevice->LightEnable(0, true);
	//DXUTToggleFullScreen();
	pd3dDevice->ShowCursor(false);
	//DXUTToggleFullScreen();


	// Creo archivo para escribir

	out.open("test.txt");
	if(!out){
		exit(1);
	}
    
	return S_OK;
}

HRESULT LoadMeshDataOptimized (IDirect3DDevice9* pd3dDevice, wchar_t* str)
{
	HRESULT hr;
	
	// Esto hay que pasarlo a una funcion que carge de manera optima una Maya
	// Genero dos Buffer para cargar las Meshes de las pelotitas
    LPD3DXMESH pMeshSysMem = NULL;			// Este buffer lo necesito para la info de la Mesh
    LPD3DXBUFFER pAdjacencyBuffer = NULL;	// Este buffer es el AdjacencyBuffer, utilizado en las optimizaciónes
    
	hr = LoadMeshData( pd3dDevice, str, &pMeshSysMem, &pAdjacencyBuffer );
    
	// If there no problems loading the mesh, proceed with the optimization (three of them).
	if( SUCCEEDED( hr ) )
    {
        hr = OptimizeMeshData( pMeshSysMem, pAdjacencyBuffer, D3DXMESHOPT_ATTRSORT, &g_MeshAttrSorted );
        if( SUCCEEDED( hr ) )
            hr = OptimizeMeshData( pMeshSysMem, pAdjacencyBuffer, D3DXMESHOPT_STRIPREORDER, &g_MeshStripReordered );

        if( SUCCEEDED( hr ) )
            hr = OptimizeMeshData( pMeshSysMem, pAdjacencyBuffer, D3DXMESHOPT_VERTEXCACHE, &g_MeshVertexCacheOptimized );

        SAFE_RELEASE( pMeshSysMem );
        SAFE_RELEASE( pAdjacencyBuffer );
    }
    else
        hr = S_OK;

	return hr;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = ( FLOAT ) pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
//    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    // update the local copies of the meshes
    UpdateLocalMeshes( pd3dDevice, &g_MeshAttrSorted );
    UpdateLocalMeshes( pd3dDevice, &g_MeshStripReordered );
    UpdateLocalMeshes( pd3dDevice, &g_MeshVertexCacheOptimized );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 200, pBackBufferSurfaceDesc->Height - 350 );
    g_SampleUI.SetSize( 200, 300 );


	// create the floor geometry
    LPD3DXMESH pMesh;
    V_RETURN( D3DXCreatePolygon( pd3dDevice, 200.0f, 4, &pMesh, NULL ) );
    V_RETURN( pMesh->CloneMeshFVF( D3DXMESH_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1,
                                   pd3dDevice, &g_pMeshFloor ) );
    SAFE_RELEASE( pMesh );

	

    DWORD dwNumVx = g_pMeshFloor->GetNumVertices();
	struct Vx
    {
        D3DXVECTOR3 vPos;
        D3DXVECTOR3 vNorm;
        float fTex[ 2 ];
    };

    // Initialize its texture coordinates
    Vx* pVx;
    hr = g_pMeshFloor->LockVertexBuffer( 0, ( VOID** )&pVx );
    if( FAILED( hr ) )
        return hr;

    for( DWORD i = 0; i < dwNumVx; ++ i )
    {
        if( fabs( pVx->vPos.x ) < 0.01 )
        {
            if( pVx->vPos.y > 0 )
            {
                pVx->fTex[ 0 ] = 0.0f;
                pVx->fTex[ 1 ] = 0.0f;
            }
            else if( pVx->vPos.y < 0.0f )
            {
                pVx->fTex[ 0 ] = 1.0f * FLOOR_TILECOUNT;
                pVx->fTex[ 1 ] = 1.0f * FLOOR_TILECOUNT;
            }
            else
            {
                pVx->fTex[ 0 ] = 0.5f * FLOOR_TILECOUNT;
                pVx->fTex[ 1 ] = 0.5f * FLOOR_TILECOUNT;
            }
        }
        else if( pVx->vPos.x > 0.0f )
        {
            pVx->fTex[ 0 ] = 1.0f * FLOOR_TILECOUNT;
            pVx->fTex[ 1 ] = 0.0f;
        }
        else
        {
            pVx->fTex[ 0 ] = 0.0f;
            pVx->fTex[ 1 ] = 1.0f * FLOOR_TILECOUNT;
        }

        ++ pVx;
    }

    g_pMeshFloor->UnlockVertexBuffer();




    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );

	tiempo = fTime - tiempoInicial;
	segundos = tiempo;

	minutos = segundos / 60;
	segundos %= 60;


	if (victory)
	{
		if (counterVicSec == -1) counterVicSec = fTime;
		if (fTime - counterVicSec > 5)
		{
			tiempoInicial = fTime;
			counterVicSec = -1;
			victory = false;
			segundos = 0;
			minutos = 0;
			gameOver = false;
			level++;
			bolas.resize(level);
		}
	}

	//if (!gameOver && !victory) gameOver = (minutos * 60 + segundos == 20*level);

	if (!victory) victory = (points >= pow(2.0, (double)level));

	if (g_Camera.IsMouseLButtonDown() && !lastMButtonState)
	{
		const D3DXVECTOR3 *dir, *pos;
		pos = g_Camera.GetEyePt();
		dir = g_Camera.GetLookAtPt();
		V3 position, direction;
		position.x = pos->x;
		position.y = pos->y;
		position.z = pos->z;

		direction.x = dir->x - pos->x;
		direction.y = dir->y - pos->y;
		direction.z = dir->z - pos->z;

		bolas.normalizar(direction);

		if (bolas.simulatePhysicsBalls(fElapsedTime, 0, g_explosion, position, direction)) points++;
	}
	else
	{
		bolas.simulatePhysicsBalls(fElapsedTime, 0, g_explosion);
	}

	lastMButtonState = g_Camera.IsMouseLButtonDown();

}

//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }
	
	if (fabs((double)(lastTime - fTime)) < 0.016f)
	{
		return;
	}

	lastTime = fTime;

    HRESULT hr;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mWorldViewProjection;

    // Clear the render target and the zbuffer
	pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 0x3F, 0xAF, 0xFF ), 1.0f, 0L );


	D3DXMATRIXA16 mx, mxView, mxProj;
	mxView = *g_Camera.GetViewMatrix();
    mxProj = *g_Camera.GetProjMatrix();
	V( pd3dDevice->SetTransform( D3DTS_VIEW, &mxView ) );
    V( pd3dDevice->SetTransform( D3DTS_PROJECTION, &mxProj ) );

	
	pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(50,50,50) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		mProj = *g_Camera.GetProjMatrix();
		mView = *g_Camera.GetViewMatrix();

		DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"Draw mesh" );
		
		double x, y, z, radio;
		x = -100.0f;
		y = 1.0f;
		z = 0.0f;

		if (!gameOver && !victory)
		{
			for( int xOffset = 0; xOffset < bolas.pballs->ballInstances; xOffset++)
			{	
				if (bolas.pballs->visible[xOffset])
				{
					x = bolas.getPositions()[xOffset].x;
					y = bolas.getPositions()[xOffset].y;
					z = bolas.getPositions()[xOffset].z;
					
					radio = bolas.getRadios()[xOffset];

					D3DXMATRIX mxx;

					D3DXMatrixScaling(&mWorld, radio, radio, radio);

					D3DXMatrixTranslation( &mxx, x, y, z );

					D3DXMatrixMultiply(&mWorld, &mWorld, &mxx);
					
					mWorldViewProjection = mWorld * mView * mProj;

					// Update the effect's variables.  Instead of using strings, it would 
					// be more efficient to cache a handle to the parameter by calling 
					// ID3DXEffect::GetParameterByName
					V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
					V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );

					DrawMeshData( pd3dDevice, g_pEffect, &g_MeshVertexCacheOptimized );
				}
			}
		}

		V( g_pEffect->SetTechnique( "RenderFloor" ) );

		// set the fixed function shader for drawing the floor
		V( pd3dDevice->SetFVF( g_pMeshFloor->GetFVF() ) );

		D3DXMATRIX mx;
		// set view-proj
		D3DXMatrixMultiply( &mx, &mView, &mProj );
		g_pEffect->SetMatrix( "g_mViewProj", &mx );
        
		//D3DXMATRIX mx;
		// floor geometry transform
	    
		D3DXMatrixRotationX( &g_mxFloor, -D3DX_PI / 2.0f );
		D3DXMatrixRotationY( &mx, D3DX_PI / 4.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		D3DXMatrixTranslation( &mx, 0.0f, 0.0f, 0.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		

		// Draw the floor
		V( g_pEffect->SetTexture( "g_txScene", g_pTxFloor ) );
		V( g_pEffect->SetMatrix( "g_mWorld", &g_mxFloor ) );
		
		UINT cPasses;
		V( g_pEffect->Begin( &cPasses, 0 ) );
		for( UINT p = 0; p < cPasses; ++p )
		{
			V( g_pEffect->BeginPass( p ) );
			V( g_pMeshFloor->DrawSubset( 0 ) );
			V( g_pEffect->EndPass() );
		}
		V( g_pEffect->End() );
		//V( pd3dDevice->EndScene() );

		//D3DXMATRIX mx;
		// floor geometry transform
	    
		D3DXMatrixRotationX( &g_mxFloor, -D3DX_PI / 0.5f );
		D3DXMatrixRotationZ( &g_mxFloor, -D3DX_PI / 4.0f );
		//D3DXMatrixRotationY( &mx, D3DX_PI / 1.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		D3DXMatrixTranslation( &mx, 0.0f, 100.0f, -100.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		

		// Draw the floor
		V( g_pEffect->SetTexture( "g_txScene", g_pTxFloor ) );
		V( g_pEffect->SetMatrix( "g_mWorld", &g_mxFloor ) );
		
		//UINT cPasses;
		V( g_pEffect->Begin( &cPasses, 0 ) );
		for( UINT p = 0; p < cPasses; ++p )
		{
			V( g_pEffect->BeginPass( p ) );
			V( g_pMeshFloor->DrawSubset( 0 ) );
			V( g_pEffect->EndPass() );
		}
		V( g_pEffect->End() );


		D3DXMatrixRotationX( &g_mxFloor, -D3DX_PI / 0.25f );
		D3DXMatrixRotationZ( &g_mxFloor, -D3DX_PI / 4.0f );
		D3DXMatrixRotationY( &mx, D3DX_PI / 1.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		D3DXMatrixTranslation( &mx, 0.0f, 100.0f, 100.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		

		// Draw the floor
		V( g_pEffect->SetTexture( "g_txScene", g_pTxFloor ) );
		V( g_pEffect->SetMatrix( "g_mWorld", &g_mxFloor ) );
		
		//UINT cPasses;
		V( g_pEffect->Begin( &cPasses, 0 ) );
		for( UINT p = 0; p < cPasses; ++p )
		{
			V( g_pEffect->BeginPass( p ) );
			V( g_pMeshFloor->DrawSubset( 0 ) );
			V( g_pEffect->EndPass() );
		}
		V( g_pEffect->End() );


		D3DXMatrixRotationX( &g_mxFloor, -D3DX_PI / 0.25f );
		D3DXMatrixRotationZ( &g_mxFloor, -D3DX_PI / 4.0f );
		D3DXMatrixRotationY( &mx, D3DX_PI / 2.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		D3DXMatrixTranslation( &mx, -100.0f, 100.0f, 0.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		

		// Draw the floor
		V( g_pEffect->SetTexture( "g_txScene", g_pTxFloor ) );
		V( g_pEffect->SetMatrix( "g_mWorld", &g_mxFloor ) );
		
		//UINT cPasses;
		V( g_pEffect->Begin( &cPasses, 0 ) );
		for( UINT p = 0; p < cPasses; ++p )
		{
			V( g_pEffect->BeginPass( p ) );
			V( g_pMeshFloor->DrawSubset( 0 ) );
			V( g_pEffect->EndPass() );
		}
		V( g_pEffect->End() );

		D3DXMatrixRotationX( &g_mxFloor, -D3DX_PI / 0.25f );
		D3DXMatrixRotationZ( &g_mxFloor, -D3DX_PI / 4.0f );
		D3DXMatrixRotationY( &mx, D3DX_PI / -2.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		D3DXMatrixTranslation( &mx, 100.0f, 100.0f, 0.0f );
		D3DXMatrixMultiply( &g_mxFloor, &g_mxFloor, &mx );
		

		// Draw the floor
		V( g_pEffect->SetTexture( "g_txScene", g_pTxFloor ) );
		V( g_pEffect->SetMatrix( "g_mWorld", &g_mxFloor ) );
		
		//UINT cPasses;
		V( g_pEffect->Begin( &cPasses, 0 ) );
		for( UINT p = 0; p < cPasses; ++p )
		{
			V( g_pEffect->BeginPass( p ) );
			V( g_pMeshFloor->DrawSubset( 0 ) );
			V( g_pEffect->EndPass() );
		}
		V( g_pEffect->End() );
		V( pd3dDevice->EndScene() );

		CDXUTPerfEventGenerator g( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
		RenderText(pd3dDevice);
		V( g_HUD.OnRender( fElapsedTime ) );
		V( g_SampleUI.OnRender( fElapsedTime ) );

		DXUT_EndPerfEvent();

	}
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText(IDirect3DDevice9* pd3dDevice)
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    
	CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    WCHAR* wszOptString;
    DWORD cTriangles = 0;
    // Calculate and show triangles per sec, a reasonable throughput number
    if( g_MeshAttrSorted.m_pMesh != NULL )
        cTriangles = g_MeshAttrSorted.m_pMesh->GetNumFaces() * g_cObjectsPerSide * g_cObjectsPerSide;
    else
        cTriangles = 0;

    float fTrisPerSec = DXUTGetFPS() * cTriangles;

    if( g_bShowVertexCacheOptimized )
        wszOptString = L"VCache Optimized";
    else if( g_bShowStripReordered )
        wszOptString = L"Strip Reordered";
    else
        wszOptString = L"Unoptimized";

    // Output statistics
    if (!gameOver)
	{
		txtHelper.Begin();
		txtHelper.SetInsertionPos( 5, 5 );
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawFormattedTextLine( L"TIEMPO   %d : %d  |  Puntaje : %d", minutos, segundos, points );
	}
	else
	{
		
		txtHelper.Begin();
		txtHelper.SetInsertionPos( 250, 250 );
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawFormattedTextLine( L"Game Over =(" );
		txtHelper.DrawFormattedTextLine( L" " );
		txtHelper.DrawFormattedTextLine( L" " );
		txtHelper.DrawFormattedTextLine( L" " );
		txtHelper.DrawFormattedTextLine( L"Puntaje Total = %d", points );
	}


	if (segundos < 3 && minutos == 0)
	{
		
		txtHelper.Begin();
		txtHelper.SetInsertionPos( 250, 250 );
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawFormattedTextLine( L"LEVEL %d", level );
	}
	
	if (victory)
	{
		
		txtHelper.Begin();
		txtHelper.SetInsertionPos( 250, 250 );
		txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		txtHelper.DrawFormattedTextLine( L"Victoly! =)" );
		txtHelper.DrawFormattedTextLine( L" " );
		txtHelper.DrawFormattedTextLine( L" " );
		txtHelper.DrawFormattedTextLine( L" " );
		txtHelper.DrawFormattedTextLine( L"Puntaje Total = %d", points );
	}


	const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        
	txtHelper.SetInsertionPos(pd3dsdBackBuffer->Width/2-4, pd3dsdBackBuffer->Height/2-4);
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"+" );

    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		{
			leftButtonPressed = true;
		}
	case WM_LBUTTONUP:
		{
			leftButtonPressed = true;
		}
	}

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;
			case VK_SPACE:
				g_explosion = true;
        }
    }
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    SAFE_RELEASE( g_pTextSprite );


	SAFE_RELEASE( g_pMeshFloor );
    g_MeshAttrSorted.ReleaseLocalMeshes();
    g_MeshStripReordered.ReleaseLocalMeshes();
    g_MeshVertexCacheOptimized.ReleaseLocalMeshes();


}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pFont );

    for( UINT i = 0; i < g_dwNumMaterials; i++ )
        SAFE_RELEASE( g_ppMeshTextures[i] );
    SAFE_DELETE_ARRAY( g_ppMeshTextures );
    SAFE_DELETE_ARRAY( g_pMeshMaterials );
	
	
    SAFE_RELEASE( g_pDefaultTex );

    g_MeshAttrSorted.ReleaseAll();
    g_MeshStripReordered.ReleaseAll();
    g_MeshVertexCacheOptimized.ReleaseAll();

    g_dwNumMaterials = 0;

	
	if( g_pVB != NULL )
        g_pVB->Release();

	SAFE_RELEASE( g_pTxFloor );
    SAFE_RELEASE( g_pMeshFloor );


	for (int i = 0; i < 1000; i++)
	{
		out << bolas.apply_sse[i] << std::endl;
	}
}



