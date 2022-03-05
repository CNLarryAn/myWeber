#pragma once
#include <string>
#include <vector>
#include <memory>

using namespace std;

struct CConfItem {
    char itemName[50];
    char itemContent[100];
};


//单例模式
class CConfig {
private:
    CConfig();
    CConfig(const CConfig&) = delete;
    CConfig& operator=(const CConfig&) = delete;
public:
    ~CConfig();

private:
    //用智能指针替换原本的内嵌类垃圾回收
    static unique_ptr<CConfig> m_instance;
    // static CConfig* m_instance;
public:
    static CConfig* GetInstence() {
        if(m_instance == nullptr) {
            //双检测，这里应该加锁保证线程安全，先不加了
            if(m_instance == nullptr) {
                m_instance.reset(new CConfig);
                // m_instance = new CConfig; 
            }
            //解锁
        }
        return m_instance.get();
    }

    //内嵌类垃圾回收，C类语言的痛
    // class CGarbo {
    // public:
    //     ~CGarbo() {
    //         if(CConfig::m_instance){
    //             delete CConfig::m_instance;
    //             CConfig::m_instance = nullptr;
    //         }
    //     }
    // };
    // static CGarbo Garbo;
    
    //好像可以用智能指针做垃圾回收

public:
    bool Load(const char *pconfName);
    const char *GetString(const char *p_itemname);
    int GetInt(const char *p_itemname, const int def);

public:
    vector<unique_ptr<CConfItem>> m_ConfigItemList;
};