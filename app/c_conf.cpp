#include "c_conf.h"
#include <string.h>

void Rtrim(char *string);
void Ltrim(char *string);

unique_ptr<CConfig> CConfig::m_instance = nullptr;

CConfig::CConfig() {
}

CConfig::~CConfig() {
}

//装载配置文件
bool CConfig::Load(const char *pconfName) 
{   
    FILE *fp;
    fp = fopen(pconfName,"r");
    if(fp == NULL)
        return false;

    //每一行配置文件读出来都放这里
    char  line_buf[501];   //每行配置都不要太长，保持<500字符内，防止出现问题
    
    //走到这里，文件打开成功 
    while(!feof(fp))  //检查文件是否结束 ，没有结束则条件成立
    {    
        //大家要注意老师的写法，注意写法的严密性，商业代码，就是要首先确保代码的严密性
        if(fgets(line_buf,500,fp) == NULL) //从文件中读数据，每次读一行，一行最多不要超过500个字符 
            continue;

        if(line_buf[0] == 0)
            continue;

        //处理注释行
        if(*line_buf==';' || *line_buf==' ' || *line_buf=='#' || *line_buf=='\t'|| *line_buf=='\n')
			continue;
        
    lblprocstring:
        //屁股后边若有换行，回车，空格等都截取掉
		if(strlen(line_buf) > 0)
		{
			if(line_buf[strlen(line_buf)-1] == 10 || line_buf[strlen(line_buf)-1] == 13 || line_buf[strlen(line_buf)-1] == 32) 
			{
				line_buf[strlen(line_buf)-1] = 0;
				goto lblprocstring;
			}		
		}
        if(line_buf[0] == 0)
            continue;
        if(*line_buf=='[') //[开头的也不处理
			continue;

        //这种 “ListenPort = 5678”走下来；

        char *ptmp = strchr(line_buf, '=');
        if(ptmp != nullptr) {
            unique_ptr<CConfItem> p_confitem(new CConfItem);
            memset(p_confitem.get(), 0, sizeof(CConfItem));
            strncpy(p_confitem->itemName, line_buf, static_cast<size_t>(ptmp - line_buf));
            strcpy(p_confitem->itemContent, ptmp + 1);

            Rtrim(p_confitem->itemName);
			Ltrim(p_confitem->itemName);
			Rtrim(p_confitem->itemContent);
			Ltrim(p_confitem->itemContent);

            m_ConfigItemList.emplace_back(move(p_confitem));
        }
    }

    fclose(fp);
    return true;
}

const char *CConfig::GetString(const char *p_itemname) {
    for(auto& p_confitem : m_ConfigItemList) { //不能用下标，也不能用迭代器，必须加引用，因为unique_ptr不能copy
        if(strcasecmp(p_confitem->itemName, p_itemname) == 0) {
            return p_confitem->itemContent;
        }
    }
    return nullptr;
}

int CConfig::GetInt(const char *p_itemname, const int def) {
    for(auto& p_confitem : m_ConfigItemList) {
        if(strcasecmp(p_confitem->itemName, p_itemname) == 0) {
            return atoi(p_confitem->itemContent);
        }
    }
    return def;
}



