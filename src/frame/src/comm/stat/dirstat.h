/*=============================================================================
#
#     FileName: dirstat.h
#         Desc: 
#
#       Author: dantezhu
#        Email: zny2008@gmail.com
#     HomePage: http://www.vimer.cn
#
#      Created: 2011-06-09 10:45:45
#      Version: 0.0.1
#      History:
#               0.0.1 | dantezhu | 2011-06-09 10:45:45 | initialization
#
=============================================================================*/
#ifndef __DIRSTAT_H__
#define __DIRSTAT_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "dir_func.h"
#include "comm_func.h"
#include "stat.h"
using namespace std;

#define STATDIR_TPL "%s/%s/%s/"

#ifndef foreach
#define foreach(container,it) \
    for(typeof((container).begin()) it = (container).begin();it!=(container).end();++it)
#endif

#ifndef isnotlast
#define isnotlast(container,it) \
    typeof(it) tmp = it; \
    tmp++; \
    if (tmp != container.end())
#endif

class CDirStat
{
public:
    CDirStat() {
        m_statDir = "./";
        m_fileName = "stat_file";
    }
    virtual ~CDirStat() {}

    int Init(const char* statDir, const char* fileName, const char * const stat_desc[],int stat_num)
    {
        m_statDir = statDir;
        m_fileName = fileName;
        m_stat_desc = stat_desc;
        m_stat_num = stat_num;

        int ret = loopLoadStatFiles();
        return ret;
    }

    int AddCount(const char* key1, const char* key2, int index)
    {
        CStatInfo* pStat = getStatFile(key1,key2);
        if (pStat == NULL)
        {
            return -1;
        }
        return pStat->AddCount(index);
    }
    int DecCount(const char* key1, const char* key2, int index)
    {
        CStatInfo* pStat = getStatFile(key1,key2);
        if (pStat == NULL)
        {
            return -1;
        }
        return pStat->DecCount(index);
    }
    int SetCount(const char* key1, const char* key2, int index,int value)
    {
        CStatInfo* pStat = getStatFile(key1,key2);
        if (pStat == NULL)
        {
            return -1;
        }
        return pStat->SetCount(index,value);
    }
    int GetCount(const char* key1, const char* key2, int index)
    {
        CStatInfo* pStat = getStatFile(key1,key2);
        if (pStat == NULL)
        {
            return -1;
        }
        return pStat->GetCount(index);
    }
    int ResetStat(const char* key1=NULL, const char* key2=NULL)
    {
        foreach(m_mapStat, it1)
        {
            if (key1 && strlen(key1)>0 && string(key1) != it1->first)
            {
                continue;
            }
            foreach(it1->second, it2)
            {
                if (key2 && strlen(key2)>0 && string(key2) != it2->first)
                {
                    continue;
                }
                if (it2->second == NULL)
                {
                    return -1;
                }
                it2->second->ResetStat();
            }
        }
        return 0;
    }
    string GetStatInfo(const char* key1=NULL, const char* key2=NULL, int num=-1)
    {
        stringstream ss;
        ss << "{" << endl;
        foreach(m_mapStat, it1)
        {
            if (key1 && strlen(key1)>0 && string(key1) != it1->first)
            {
                continue;
            }
            ss << "\t\"" << it1->first << "\":{" << endl;
            foreach(it1->second, it2)
            {
                if (key2 && strlen(key2)>0 && string(key2) != it2->first)
                {
                    continue;
                }
                if (it2->second == NULL)
                {
                    return "";
                }
                ss << "\t\t\"" << it2->first << "\":{" << endl;

                vector<pair<string,int> > _map = it2->second->GetStatMap();
                foreach(_map, it3)
                {
                    ss << "\t\t\t\"" << it3->first << "\":" << it3->second;
                    isnotlast(_map, it3)
                    {
                        ss << ",";
                    }
                    ss << endl;
                }
                ss << "\t\t}";

                if (!(key2 && strlen(key2)>0))
                {
                    isnotlast(it1->second, it2)
                    {
                        ss << ",";
                    }
                }
                ss << endl;
            }
            ss << "\t}";

            if (!(key1 && strlen(key1)>0))
            {
                isnotlast(m_mapStat, it1)
                {
                    ss << ",";
                }
            }
            ss << endl;
        }
        ss << "}" << endl;
        return ss.str();
    }

private:
    int loopLoadStatFiles()
    {
        vector<string> vecFiles;
        int ret;

        ret = GetFileInDir(m_statDir, m_fileName, vecFiles);
        if (ret != 0)
        {
            return -1;
        }

        foreach(vecFiles, it)
        {
            ret = loadStatFile(*it);
            if (ret != 0)
            {
                return -2;
            }
        }
        return 0;
    }

    CStatInfo* getStatFile(const char* key1, const char* key2)
    {
        int ret;
        CStatInfo*& pStat = m_mapStat[key1][key2];
        if (pStat == NULL)
        {
            snprintf(m_szBuf,sizeof(m_szBuf),STATDIR_TPL,m_statDir.c_str(),key1,key2);
            //需要递归创建目录
            ret = mkdirs(m_szBuf);
            if (ret != 0)
            {
                return NULL;
            }
            //文件路径
            strncat(m_szBuf,m_fileName.c_str(), m_fileName.size());
            pStat = new CStatInfo();
            pStat->Init(m_szBuf, m_stat_desc, m_stat_num);
        }

        return pStat;
    }

    int loadStatFile(string filePath)
    {
        string::size_type pos = filePath.find(m_statDir);
        if (pos != string::npos)
        {
            filePath.replace(pos, m_statDir.size(), "");
        }
        pos = filePath.rfind(m_fileName);
        if (pos != string::npos)
        {
            filePath.replace(pos, m_fileName.size(), "");
        }

        vector<string> destVec;
        SplitString(filePath,"/",destVec);

        vector<string> vecNew;
        foreach(destVec, it)
        {
            if (it->empty())
            {
                continue;
            }
            vecNew.push_back(*it);
        }
        if (vecNew.size() != 2)
        {
            return -1;
        }

        CStatInfo * pStat = getStatFile(vecNew[0].c_str(),vecNew[1].c_str());
        if (pStat == NULL)
        {
            return -2;
        }

        return 0;
    }

private:
    string m_statDir;
    string m_fileName;
    char m_szBuf[1024];
    map<string, map<string, CStatInfo* > > m_mapStat;

    const char * const * m_stat_desc;
    int         m_stat_num;
};
#endif
