#ifndef MODPTR_H
#define MODPTR_H

template <class T>
class ModPtr {
    ModManager::Proxy *proxy;
    T* obj;
public:
    inline ModPtr() : proxy(0), obj(0) { /*ls_error("ModPtr at 0x%x\n", this);*/}

    inline ModPtr(ModManager::ModMan *mm, const char *ifname, bool instanciate=false) {
        ModManager::ModList list;
        ModManager::IUnknown *unk;
        mm->queryModulesByInterface(ifname, list);
        assert(list.size() > 0);
        proxy = list.back();
        if (instanciate) {
            proxy = proxy->instanciate();
        }
        assert(proxy);
        //ls_message("[BEGIN: ModPtr(mm=0x%x, ifname=%s)]\n", mm, ifname);
        //ls_message("  proxy is at: 0x%x\n", proxy);
        unk = proxy->ref();
        assert(unk);
        //ls_message("  IUnknown object is at: 0x%x\n", unk);
        //ls_message("  %s Implementation name is %s\n",
        //        ifname,
        //        unk->queryInformation()->name);
        obj = dynamic_cast<T*> (unk);
        //ls_message("  %s object is at: 0x%x\n", ifname, obj);
        assert(obj);
        //ls_message("[END]\n");
    }
    
    inline ModPtr(const ModPtr<T> & src) {
        this->proxy=src.proxy;
        assert(proxy);
        ModManager::IUnknown * unk=proxy->ref();
        assert(unk);
        obj = dynamic_cast<T*> (unk);
        assert(obj);
    }
    
    inline ModPtr(ModManager::Proxy *proxy) {
        ModManager::IUnknown *unk;
        this->proxy=proxy;
        assert(proxy);
        unk=proxy->ref();
        assert(unk);
        obj = dynamic_cast<T*> (unk);
        assert(obj);
    }
    
    inline ~ModPtr() {
        if (proxy) proxy->unref();
    }
    
    inline ModPtr instanciate() {
        return ModPtr(proxy->instanciate());
    }
    
    inline void operator= (const ModPtr<T> &src) {
        if (src.proxy) src.proxy->ref();
        if (proxy) proxy->unref();
        proxy = src.proxy;
        obj = src.obj;
    }
    
    inline void operator= (ModManager::Proxy *proxy) {
        ModManager::IUnknown *unk;

        if (this->proxy) this->proxy->unref();
        this->proxy=proxy;
        ls_warning("proxy=0x%x\n", proxy);
        assert(proxy);
        unk = proxy->ref();
        ls_warning("unk=0x%x\n", unk);
        obj = dynamic_cast<T*> (unk);
        ls_warning("obj=0x%x\n", obj);
        assert(obj);
    }
        
    
    inline T* operator->() {
        assert(obj);
        return obj;
    }
    
    inline T& operator*() {
        assert(obj);
        return *obj;
    }
    
    inline int isValid() {
        return (obj)?1:0;
    }
    
    inline ModManager::Proxy *getProxy() {
        return proxy;
    }
};

#endif
