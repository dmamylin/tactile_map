#ifndef CWIC_IMAGING_FACTORY
#define CWIC_IMAGING_FACTORY

#include <memory>

class CWICImagingFactory {
private:
    CComPtr<IWICImagingFactory> m_pImagingFactory;
    static std::shared_ptr<CWICImagingFactory> m_pInstance;

    CWICImagingFactory() :
    m_pImagingFactory(nullptr) {
        CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory, (LPVOID*)&m_pImagingFactory);
    }

public:
    static CWICImagingFactory& getInstance();

    IWICImagingFactory* getFactory() const {
        return m_pImagingFactory;
    }
};

std::shared_ptr<CWICImagingFactory> CWICImagingFactory::m_pInstance;

CWICImagingFactory& CWICImagingFactory::getInstance(){
    if (m_pInstance.get() == nullptr) {
        m_pInstance.reset(new CWICImagingFactory());
    }
    return *m_pInstance;
}

#endif
