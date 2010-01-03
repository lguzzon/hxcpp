#include <hxcpp.h>
#include "FieldMap.h"

namespace hx
{

Anon_obj::Anon_obj()
{
   mFields = hx::FieldMapCreate();
}

void Anon_obj::__Mark()
{
	// We will get mFields=0 here if we collect in the constructor before mFields is assigned
	if (mFields)
	{
		hx::MarkAlloc(mFields);
		hx::FieldMapMark(mFields);
	}
}



Dynamic Anon_obj::__Field(const String &inString)
{
   Dynamic *v = mFields->Find(inString);
	if (!v)
      return null();
   return *v;
}

bool Anon_obj::__HasField(const String &inString)
{
   return mFields->Find(inString);
}


bool Anon_obj::__Remove(String inKey)
{
	return mFields->Erase(inKey);
}


Dynamic Anon_obj::__SetField(const String &inString,const Dynamic &inValue)
{
	mFields->Insert(inString,inValue);
   return inValue;
}

Anon_obj *Anon_obj::Add(const String &inName,const Dynamic &inValue)
{
	mFields->Insert(inName,inValue);
   if (inValue.GetPtr())
		inValue.GetPtr()->__SetThis(this);
   return this;
}

struct Stringer
{
	Stringer(String &inString) : result(inString), first(true) { }
	void Visit(void *, const String &inStr, Dynamic &inDyn)
	{
		if (first)
      {
         result += inStr + HX_STRING(L" => ",4) + (String)(inDyn);
         first = false;
      }
      else
         result += HX_STRING(L", ",2) + inStr + HX_STRING(L" => ",4) + (String)(inDyn);
	}

	bool first;
	String &result;
};

String Anon_obj::toString()
{
   String result = HX_STRING(L"{ ",2);
	Stringer stringer(result);
	mFields->Iterate(stringer);
   return result + HX_STRING(L" }",2);
}

void Anon_obj::__GetFields(Array<String> &outFields)
{
	KeyGetter getter(outFields);
	mFields->Iterate(getter);
}



String Anon_obj::__ToString() const { return HX_STR(L"Anon"); }


Dynamic Anon_obj::__Create(DynamicArray inArgs) { return Anon(new Anon_obj); }

Class Anon_obj::__mClass;


void Anon_obj::__boot()
{
   Static(__mClass) = RegisterClass(HX_STR(L"__Anon"),TCanCast<Anon_obj>,sNone,sNone,0,0,0,0);
}

}



bool __hxcpp_anon_remove(hx::Anon inObj,String inKey)
{
	return inObj->__Remove(inKey);
}


