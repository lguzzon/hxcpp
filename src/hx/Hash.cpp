#include <hxcpp.h>
#include "RedBlack.h"
#include "FieldMap.h"

#ifndef HX_INTERNAL_GC
#include <gc_allocator.h>
#else
#include <hx/CFFI.h>
#endif
#include <map>


using namespace hx;

// --- IntHash ------------------------------------------------------------------------------

namespace hx
{

// --- FieldObject ------------------------------

FieldMap *FieldMapCreate()
{
	return (FieldMap *)(RBTree<String,Dynamic>::Create());
}

bool FieldMapGet(FieldMap *inMap, const String &inName, Dynamic &outValue)
{
	Dynamic *value = inMap->Find(inName);
	if (!value)
		return false;
	outValue = *value;
	return true;
}

bool FieldMapGet(FieldMap *inMap, int inID, Dynamic &outValue)
{
	Dynamic *value = inMap->Find(__hxcpp_field_from_id(inID));
	if (!value)
		return false;
	outValue = *value;
	return true;
}

void FieldMapSet(FieldMap *inMap, const String &inName, const Dynamic &inValue)
{
	inMap->Insert(inName,inValue);
}



void FieldMapAppendFields(FieldMap *inMap,Array<String> &outFields)
{
	KeyGetter getter(outFields);
	inMap->Iterate(getter);
}

struct Marker
{
	void Visit(void *inPtr, const String &inStr, Dynamic &inDyn)
	{
		hx::MarkAlloc(inPtr);
		HX_MARK_STRING(inStr.__s);
		if (inDyn.mPtr)
		   HX_MARK_OBJECT(inDyn.mPtr);
	}
};

void FieldMapMark(FieldMap *inMap)
{
	Marker m;
	inMap->Iterate(m);
}






class IntHash : public Object
{
#ifdef _WIN32
typedef int IntKey;
#else
typedef const int IntKey;
#endif

#ifdef HX_INTERNAL_GC
typedef std::map<IntKey,Dynamic, std::less<IntKey> > Map;
#else
typedef std::map<IntKey,Dynamic, std::less<IntKey>, gc_allocator< std::pair<IntKey, Dynamic> > > Map;
#endif

class hxMap : public Map
{
#ifndef HX_INTERNAL_GC
public:
   void *operator new( size_t inSize ) { return GC_MALLOC(inSize); }
   void operator delete( void * ) { }
#endif
};

private:
   Map *mMap;
#ifdef HX_INTERNAL_GC
	hx::InternalFinalizer *mFinalizer;
#endif

public:
   IntHash()
	{
		mMap = new hxMap;
		#ifdef HX_INTERNAL_GC
		mFinalizer = new hx::InternalFinalizer(this);
		mFinalizer->mFinalizer = Destroy;
		#endif
	}

   void set(int inKey,const Dynamic &inValue) { (*mMap)[inKey] = inValue; }

   static void Destroy(Object *inObj);

   Dynamic get(int inKey)
   {
      Map::iterator i = mMap->find(inKey);
      if (i==mMap->end()) return null();
      return i->second;
   }
   bool exists(int inKey) { return mMap->find(inKey)!=mMap->end(); }
   bool remove(int inKey)
   {
      Map::iterator i = mMap->find(inKey);
      if (i==mMap->end()) return false;
      mMap->erase(i);
      return true;
   }
   Dynamic keys()
   {
      Array<Int> result(0,mMap->size());
      for(Map::iterator i=mMap->begin();i!=mMap->end();++i)
         result.Add(i->first);
      return result;
   }
   Dynamic values()
   {
      Array<Dynamic> result(0,mMap->size());
      for(Map::iterator i=mMap->begin();i!=mMap->end();++i)
         result.Add(i->second);
      return result;
   }
   String toString()
   {
      String result = L"{ ";
      Map::iterator i=mMap->begin();
      while(i!=mMap->end())
      {
         result += String(i->first) + String(L" => ",1) + i->second;
         ++i;
         if (i!=mMap->end())
            result+= L",";
      }

      return result + L"}";
   }

   void __Mark()
   {
		#ifdef HX_INTERNAL_GC
		mFinalizer->Mark();
		#endif
      for(Map::iterator i=mMap->begin();i!=mMap->end();++i)
      {
         HX_MARK_OBJECT(i->second.mPtr);
      }
   }

};

void IntHash::Destroy(Object *inHash)
{
	IntHash *hash = dynamic_cast<IntHash *>(inHash);
	if (hash)
		delete hash->mMap;
}

}


Object * __int_hash_create() { return new IntHash; }

void __int_hash_set(Dynamic &inHash,int inKey,const Dynamic &value)
{
   IntHash *h = dynamic_cast<IntHash *>(inHash.GetPtr());
   h->set(inKey,value);
}

Dynamic  __int_hash_get(Dynamic &inHash,int inKey)
{
   IntHash *h = dynamic_cast<IntHash *>(inHash.GetPtr());
   return h->get(inKey);
}

bool  __int_hash_exists(Dynamic &inHash,int inKey)
{
   IntHash *h = dynamic_cast<IntHash *>(inHash.GetPtr());
   return h->exists(inKey);
}

bool  __int_hash_remove(Dynamic &inHash,int inKey)
{
   IntHash *h = dynamic_cast<IntHash *>(inHash.GetPtr());
   return h->remove(inKey);
}

Dynamic __int_hash_keys(Dynamic &inHash)
{
   IntHash *h = dynamic_cast<IntHash *>(inHash.GetPtr());
   return h->keys();
}

Dynamic __int_hash_values(Dynamic &inHash)
{
   IntHash *h = dynamic_cast<IntHash *>(inHash.GetPtr());
   return h->values();
}
