#ifndef MOD_MORPHSUMMON_H
#define MOD_MORPHSUMMON_H

#include "ScriptMgr.h"

class MorphSummonScriptMgr
{
    friend class ACE_Singleton<MorphSummonScriptMgr, ACE_Null_Mutex>;

    public:
        void OnAfterPolymorph(Player* player, Pet* pet, uint32 spell, bool polymorphPet, uint32 morphId);
};

#define sMorphSummonScriptMgr ACE_Singleton<MorphSummonScriptMgr, ACE_Null_Mutex>::instance()

class MorphSummonModuleScript : public ModuleScript
{
    public:
        virtual void OnAfterPolymorph(Player* /*player*/, Pet* /*pet*/, uint32 /*spell*/, bool /*polymorphPet*/, uint32 /*morphId*/) { }

    protected:
        MorphSummonModuleScript(const char* name);
};

template class ScriptRegistry<MorphSummonModuleScript>;

#endif
