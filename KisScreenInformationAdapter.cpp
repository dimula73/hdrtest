#include "KisScreenInformationAdapter.h"

#include "kis_debug.h"
#include <QOpenGLContext>

//#include <QtGui/5.11.2/QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/5.12.0/QtGui/qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include <QWindow>
#include <d3d11.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include "EGL/egl.h"
#include "EGL/eglext.h"


namespace {
struct EGLException {
    EGLException() {}
    EGLException(const QString &what) : m_what(what) {}

    QString what() const {
        return m_what;
    }

private:
    QString m_what;
};

template <typename FuncType>
void getProcAddressSafe(QOpenGLContext *context, const char *funcName, FuncType &func)
{
    func = reinterpret_cast<FuncType>(context->getProcAddress(funcName));
    if (!func) {
        throw EGLException(QString("failed to fetch function %1").arg(funcName));
    }
}

typedef const char *(EGLAPIENTRYP PFNEGLQUERYSTRINGPROC) (EGLDisplay dpy, EGLint name);
}


struct KisScreenInformationAdapter::Private
{
    void initialize(QOpenGLContext *context);

    QOpenGLContext *context;
    QString errorString;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter;
};

KisScreenInformationAdapter::KisScreenInformationAdapter(QOpenGLContext *context)
    : m_d(new Private)
{
    m_d->initialize(context);
}

KisScreenInformationAdapter::~KisScreenInformationAdapter()
{
}

void KisScreenInformationAdapter::Private::initialize(QOpenGLContext *newContext)
{
    context = newContext;
    errorString.clear();

    try {

        if (!context->isOpenGLES()) {
            throw EGLException("the context is not OpenGL ES");
        }

        PFNEGLQUERYSTRINGPROC queryString = nullptr;
        getProcAddressSafe(context, "eglQueryString", queryString);

        const char* client_extensions = queryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        const QList<QByteArray> extensions = QByteArray(client_extensions).split(' ');

        if (!extensions.contains("EGL_ANGLE_platform_angle_d3d") ||
            !extensions.contains("EGL_ANGLE_device_creation_d3d11")) {

            throw EGLException("the context is not Angle + D3D11");
        }

        PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttribEXT = nullptr;
        PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttribEXT = nullptr;

        getProcAddressSafe(context, "eglQueryDisplayAttribEXT", queryDisplayAttribEXT);
        getProcAddressSafe(context, "eglQueryDeviceAttribEXT", queryDeviceAttribEXT);

        QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
        EGLDisplay display = reinterpret_cast<EGLDisplay>(nativeInterface->nativeResourceForContext("egldisplay", context));

        if (!display) {
            throw EGLException(
                QString("couldn't request EGLDisplay handle, display = 0x%1").arg(uintptr_t(display), 0, 16));
        }

        EGLAttrib value = 0;
        EGLBoolean result = false;

        result = queryDisplayAttribEXT(display, EGL_DEVICE_EXT, &value);

        if (!result || value == EGL_NONE) {
            throw EGLException(
               QString("couldn't request EGLDeviceEXT handle, result = 0x%1, value = 0x%2")
                   .arg(result, 0, 16).arg(value, 0, 16));
        }

        EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(value);

        result = queryDeviceAttribEXT(device, EGL_D3D11_DEVICE_ANGLE, &value);

        if (!result || value == EGL_NONE) {
            throw EGLException(
                QString("couldn't request ID3D11Device pointer, result = 0x%1, value = 0x%2")
                    .arg(result, 0, 16).arg(value, 0, 16));
        }
        ID3D11Device *deviceD3D = reinterpret_cast<ID3D11Device*>(value);

        {
            HRESULT result = 0;

            Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
            result = deviceD3D->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

            if (FAILED(result)) {
                throw EGLException(
                    QString("couldn't request IDXGIDevice pointer, result = 0x%1").arg(result, 0, 16));
            }

            Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter;
            result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter1), (void**)&dxgiAdapter);

            if (FAILED(result)) {
                throw EGLException(
                    QString("couldn't request IDXGIAdapter1 pointer, result = 0x%1").arg(result, 0, 16));
            }

            this->dxgiAdapter = dxgiAdapter;
        }
    } catch (EGLException &e) {
        this->context = 0;
        this->errorString = e.what();
        this->dxgiAdapter.Reset();
    }
}

bool KisScreenInformationAdapter::isValid() const
{
    return m_d->context && m_d->dxgiAdapter;
}

QString KisScreenInformationAdapter::errorString() const
{
    return m_d->errorString;
}

KisScreenInformationAdapter::ScreenInfo KisScreenInformationAdapter::infoForScreen(QScreen *screen) const
{
    ScreenInfo info;

    QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
    HMONITOR monitor = reinterpret_cast<HMONITOR>(nativeInterface->nativeResourceForScreen("handle", screen));

    if (!monitor) {
        qWarning("%s: failed to get HMONITOR handle for screen: screen = 0x%X, monitor = 0x%X",
                 __PRETTY_FUNCTION__, screen, monitor);
    }

    UINT i = 0;
    Microsoft::WRL::ComPtr<IDXGIOutput> currentOutput;

    while (m_d->dxgiAdapter->EnumOutputs(i, &currentOutput) != DXGI_ERROR_NOT_FOUND)
    {

        HRESULT result = 0;
        Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
        result = currentOutput.As(&output6);

        if (output6) {
            DXGI_OUTPUT_DESC1 desc;
            result = output6->GetDesc1(&desc);

            if (desc.Monitor == monitor) {
                info.screen = screen;
                info.bitsPerColor = desc.BitsPerColor;
                info.redPrimary[0] = desc.RedPrimary[0];
                info.redPrimary[1] = desc.RedPrimary[1];
                info.greenPrimary[0] = desc.GreenPrimary[0];
                info.greenPrimary[1] = desc.GreenPrimary[1];
                info.bluePrimary[0] = desc.BluePrimary[0];
                info.bluePrimary[1] = desc.BluePrimary[1];
                info.whitePoint[0] = desc.WhitePoint[0];
                info.whitePoint[1] = desc.WhitePoint[1];
                info.minLuminance = desc.MinLuminance;
                info.maxLuminance = desc.MaxLuminance;
                info.maxFullFrameLuminance = desc.MaxFullFrameLuminance;

                info.colorSpace = QSurfaceFormat::DefaultColorSpace;

                if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) {
                    info.colorSpace = QSurfaceFormat::sRGBColorSpace;
                } else if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) {
                    info.colorSpace = QSurfaceFormat::scRGBColorSpace;
                } else if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
                    info.colorSpace = QSurfaceFormat::bt2020PQColorSpace;
                } else {
                    qWarning("WARNING: unknown display color space! 0x%X", desc.ColorSpace);
                }

                break;
            }
        }

        i++;
    }

    return info;
}

QDebug operator<<(QDebug dbg, const KisScreenInformationAdapter::ScreenInfo &info)
{
    QDebugStateSaver saver(dbg);

    if (info.isValid()) {
        dbg.nospace() << "ScreenInfo("
                      << "screen " << info.screen
                      << ", bitsPerColor " << info.bitsPerColor
                      << ", colorSpace " << info.colorSpace
                      << ", redPrimary " << "(" << info.redPrimary[0] << ", " << info.redPrimary[1] << ")"
                      << ", greenPrimary " << "(" << info.greenPrimary[0] << ", " << info.greenPrimary[1] << ")"
                      << ", bluePrimary " << "(" << info.bluePrimary[0] << ", " << info.bluePrimary[1] << ")"
                      << ", whitePoint " << "(" << info.whitePoint[0] << ", " << info.whitePoint[1] << ")"
                      << ", minLuminance " << info.minLuminance
                      << ", maxLuminance " << info.maxLuminance
                      << ", maxFullFrameLuminance " << info.maxFullFrameLuminance
                      << ')';
    } else {
        dbg.nospace() << "ScreenInfo(<invalid>)";
    }

    return dbg;
}
