//$Id: RefData.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  RefData
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2004/01/09
//
/**
 * Declares base class of reference data.
 */
//------------------------------------------------------------------------------
#ifndef RefData_hpp
#define RefData_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"


/**
 * Moved out of the RefObjType class so that the Visual Studio exports could be implemented
 */
struct GMAT_API RefObjType
{
   Gmat::ObjectType objType;
   wxString objName;
   GmatBase *obj;

   // Constructor -- default values required for the DevStudio export issues 
   RefObjType(Gmat::ObjectType refType = Gmat::UNKNOWN_OBJECT, 
      const wxString &refName = wxT(""), GmatBase *ref = NULL)
      {
         objType = refType;
         objName = refName;
         obj     = ref;
      };

   RefObjType& operator= (const RefObjType& right)
      {
         if (this == &right)
            return *this;
         objType = right.objType;
         objName = right.objName;
         obj     = right.obj;
         return *this;
      };
};

#ifdef EXPORT_TEMPLATES

    // Instantiate STL template classes used in GMAT  
    // This does not create an object. It only forces the generation of all
    // of the members of the listed classes. It exports them from the DLL 
    // and imports them into the .exe file.

    // This fixes wxString:
    EXPIMP_TEMPLATE template class DECLSPECIFIER std::allocator<RefObjType>;
    EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<RefObjType>;

#endif

class GMAT_API RefData
{
public:

   RefData(const wxString &name = wxT(""));
   RefData(const RefData &rd);
   RefData& operator= (const RefData &rd);
   virtual ~RefData();
   
   GmatBase* GetSpacecraft();
   
   Integer GetNumRefObjects() const;
   
   wxString GetRefObjectName(const Gmat::ObjectType type) const;
   const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type);
   
   bool SetRefObjectName(const Gmat::ObjectType type,
                         const wxString &name);
   GmatBase* GetRefObject(const Gmat::ObjectType type,
                          const wxString &name = wxT(""));
   bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                     const wxString &name = wxT(""));

   bool RenameRefObject(const Gmat::ObjectType type,
                        const wxString &oldName,
                        const wxString &newName);
   
   virtual bool ValidateRefObjects(GmatBase *param) = 0;
   virtual const wxString* GetValidObjectList() const;

protected:

   //struct RefObjType
   //{
   //   Gmat::ObjectType objType;
   //   wxString objName;
   //   GmatBase *obj;
   //   RefObjType(Gmat::ObjectType refType, const wxString &refName, GmatBase *ref)
   //      {
   //         objType = refType;
   //         objName = refName;
   //         obj     = ref;
   //      };
   //   RefObjType& operator= (const RefObjType& right)
   //      {
   //         if (this == &right)
   //            return *this;
   //         objType = right.objType;
   //         objName = right.objName;
   //         obj     = right.obj;
   //         return *this;
   //      };
   //};
   
   wxString mName;
   std::vector<RefObjType> mRefObjList;
   
   StringArray mObjectTypeNames;
   StringArray mAllRefObjectNames;
   Integer mNumRefObjects;
   
   bool AddRefObject(const Gmat::ObjectType type,
                     const wxString &name, GmatBase *obj = NULL,
                     bool replaceName = false);
   
   bool SetRefObjectWithNewName(GmatBase *obj, const Gmat::ObjectType type,
                                const wxString &name);
   
   bool HasObjectType(const wxString &type) const;
   GmatBase* FindFirstObject(const wxString &type) const;
   GmatBase* FindFirstObject(const Gmat::ObjectType type) const;
   wxString FindFirstObjectName(const Gmat::ObjectType type) const;
   
   virtual void InitializeRefObjects();
   virtual bool IsValidObjectType(Gmat::ObjectType type) = 0;
};
#endif // RefData_hpp

