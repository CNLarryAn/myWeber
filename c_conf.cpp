#include "c_conf.h"
#include <string.h>

shared_ptr<CConfig> CConfig::m_instance;

CConfig::CConfig() {
}

CConfig::~CConfig() {
}

bool CConfig::Load(const char *pconfName) {
    FILE *fp;
    fp = fopen(pconfName, "r");
    if(fp == nullptr) {
        return false;
    }

    char line_buf[501];
    while(!feof(fp)) {
        if(fgets(line_buf, 500, fp) == nullptr) {
            continue;
        }

        if(line_buf[0] == 0) {
            continue;
        }

        char *ptmp = strchr(line_buf, '=');
        if(ptmp != nullptr) {
            shared_ptr<CConfItem> p_confitem(new CConfItem);
            memset(p_confitem.get(), 0, sizeof(CConfItem));
            strncpy(p_confitem->itemName, line_buf, static_cast<size_t>(ptmp - line_buf));
            strcpy(p_confitem->itemContent, ptmp + 1);

            m_ConfigItemList.push_back(p_confitem);
        }
    }

    fclose(fp);
    return true;
}

const char *CConfig::GetString(const char *p_itemname) {
    for(int i = 0; i < m_ConfigItemList.size(); ++i) {
        if(strcasecmp(m_ConfigItemList[i]->itemName, p_itemname) == 0) {
            return m_ConfigItemList[i]->itemContent;
        }
    }
    return nullptr;
}

int CConfig::GetInt(const char *p_itemname, const int def) {
    for(int i = 0; i < m_ConfigItemList.size(); ++i) {
        if(strcasecmp(m_ConfigItemList[i]->itemName, p_itemname) == 0) {
            return atoi(m_ConfigItemList[i]->itemContent);
        }
    }
    return def;
}
