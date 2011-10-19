//$Id: MathElement.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   MathElement
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Waka Waktola
// Created: 2006/04/04
//
/**
 * Defines the Math elements class for Math in scripts.
 */
//------------------------------------------------------------------------------

#include "MathElement.hpp"
#include "StringUtil.hpp"  // for ToReal()
#include "MessageInterface.hpp"
#include <sstream>

//#define DEBUG_MATH_ELEMENT
//#define DEBUG_WRAPPERS
//#define DEBUG_INPUT_OUTPUT
//#define DEBUG_EVALUATE
//#define DEBUG_RENAME

//------------------------------------------------------------------------------
//  MathElement(wxString typeStr, wxString name)
//------------------------------------------------------------------------------
/**
 * Constructs the MathElement object (default constructor).
 * 
 * @param <typeStr> String text identifying the object type
 * @param <name>   Name for the object
 */
//------------------------------------------------------------------------------
MathElement::MathElement(const wxString &typeStr, const wxString &name) :
   MathNode      (wxT("MathElement"), name),
   refObject     (NULL),
   refObjectName (wxT(""))
{
   #ifdef DEBUG_MATH_ELEMENT
   MessageInterface::ShowMessage
      (wxT("MathElement::MathElement() typeStr='%s', name='%s' entered\n"), typeStr.c_str(),
       name.c_str());
   #endif
   
   isFunction = false;
   objectTypeNames.push_back(wxT("MathElement"));   
   theWrapperMap = NULL;
   
   Real rval;
   if (GmatStringUtil::ToReal(name, &rval))
   {
      SetRealValue(rval);
      SetNumberFlag(true);
   }
   else
   {
      SetRefObjectName(Gmat::PARAMETER, name);
   }
   
   #ifdef DEBUG_MATH_ELEMENT
   MessageInterface::ShowMessage
      (wxT("MathElement::MathElement() created\n"));
   #endif
}


//------------------------------------------------------------------------------
//  ~MathElement(void)
//------------------------------------------------------------------------------
/**
 * Destroys the MathElement object (destructor).
 */
//------------------------------------------------------------------------------
MathElement::~MathElement()
{
}


//------------------------------------------------------------------------------
//  MathElement(const MathElement &me)
//------------------------------------------------------------------------------
/**
 * Constructs the MathElement object (copy constructor).
 * 
 * @param <me> Object that is copied
 */
//------------------------------------------------------------------------------
MathElement::MathElement(const MathElement &me) :
   MathNode      (me),
   refObject     (me.refObject),
   refObjectName (me.refObjectName),
   refObjectType (me.refObjectType)
{
}


//------------------------------------------------------------------------------
//  MathElement& operator=(const MathElement &me)
//------------------------------------------------------------------------------
/**
 * Sets one MathElement object to match another (assignment operator).
 * 
 * @param <me> The object that is copied.
 * 
 * @return this object, with the parameters set as needed.
 */
//------------------------------------------------------------------------------
MathElement& MathElement::operator=(const MathElement &me)
{
   if (this == &me)
      return *this;
   
   MathNode::operator=(me);
   refObject = me.refObject;
   refObjectName = me.refObjectName;
   refObjectType = me.refObjectType;
   
   return *this;
}


//------------------------------------------------------------------------------
// void SetMathWrappers(std::map<wxString, ElementWrapper*> *wrapperMap)
//------------------------------------------------------------------------------
void MathElement::SetMathWrappers(std::map<wxString, ElementWrapper*> *wrapperMap)
{
   theWrapperMap = wrapperMap;
   
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathElement::SetMathWrappers() theWrapperMap=%p, Node=%s\n"),
       theWrapperMap, GetName().c_str());
   #endif
}


// Inherited MathElement methods
//------------------------------------------------------------------------------
// virtual void SetMatrixValue(const Rmatrix &mat)
//------------------------------------------------------------------------------
void MathElement::SetMatrixValue(const Rmatrix &mat)
{
   // if we can set matrix value, it is not a parameter
   isNumber = true;
   refObjectName = wxT("");

   MathNode::SetMatrixValue(mat);
}


//------------------------------------------------------------------------------
// void GetOutputInfo()
//------------------------------------------------------------------------------
void MathElement::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("MathElement::GetOutputInfo() this=<%p><%s><%s>\n"), this, GetTypeName().c_str(),
       GetName().c_str());
   #endif
   
   type = elementType;
   rowCount = 1;
   colCount = 1;
   
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("MathElement::GetOutputInfo() isNumber=%d, isFunctionInput=%d, refObjectName=%s\n"),
       isNumber, isFunctionInput, refObjectName.c_str());
   #endif
   
   // if this this function input, just return
   if (isFunctionInput)
      return;
   
   if (refObjectName == wxT(""))
   {
      if (type == Gmat::RMATRIX_TYPE)
      {
         rowCount = matrix.GetNumRows();
         colCount = matrix.GetNumColumns();
      }
   }
   else
   {
      #ifdef DEBUG_INPUT_OUTPUT
      MessageInterface::ShowMessage
         (wxT("MathElement::GetOutputInfo() %s is parameter\n"), GetName().c_str());
      #endif
      
      if (refObject)
      {
         type = refObject->GetReturnType();
         
         if (type == Gmat::RMATRIX_TYPE)
         {
            // Handle array index
            wxString newName, rowStr, colStr;
            GmatStringUtil::GetArrayIndexVar(refObjectName, rowStr, colStr, newName, wxT("()"));
            #ifdef DEBUG_INPUT_OUTPUT
            MessageInterface::ShowMessage
               (wxT("   rowStr='%s', colStr='%s', newName=%s\n"), rowStr.c_str(),
                colStr.c_str(), newName.c_str());
            #endif
            
            // Are we going to allow row/column slicing for future? such as:
            // a(:,1)   -> first column vector
            // a(1,:)   -> first row vector
            // a(1:2,1) -> first and second row, fisrt column vector
            
            bool wholeArray = false;
            if (rowStr == wxT("-1") && colStr == wxT("-1"))
               wholeArray = true;
            
            // if whole array, row and colum count is actual dimension
            if (wholeArray)
            {
               rowCount = ((Array*)refObject)->GetRowCount();
               colCount = ((Array*)refObject)->GetColCount();
            }
            else
            {
               type = Gmat::REAL_TYPE;
               rowCount = 1;
               colCount = 1;
            }
         }
         
         #ifdef DEBUG_INPUT_OUTPUT
         MessageInterface::ShowMessage
            (wxT("MathElement::GetOutputInfo() type=%d, row=%d, col=%d\n"), type,
             rowCount, colCount);
         #endif
      }
      else
      {
         throw MathException(wxT("The output parameter: ") + GetName() + wxT(" is NULL"));
      }
   }
}


//------------------------------------------------------------------------------
// bool ValidateInputs()
//------------------------------------------------------------------------------
/**
 * This method always returns true if the node was sucessfully initialized.
 */
 //------------------------------------------------------------------------------
bool MathElement::ValidateInputs()
{
   // The function inputs shoule be handled by the FunctionRunner
   if (isFunctionInput)
      return false;
   
   if (elementType == Gmat::REAL_TYPE && refObjectName == wxT(""))
      return true;
   
   if (refObject)
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// Real Evaluate()
//------------------------------------------------------------------------------
Real MathElement::Evaluate()
{
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("MathElement::Evaluate() this='%s', refObjectName='%s', refObject=<%p>, ")
       wxT("elementType=%d\n"), GetName().c_str(), refObjectName.c_str(), refObject, elementType);
   #endif
   
   // If this MathElement is function Input, just return since it is handled in
   // the FunctionRunner
   
   if (isFunctionInput)
      throw MathException(wxT("MathElement::Evaluate() Function input should ")
                          wxT("not be handled here"));
   
   if (refObject)
   {
      #ifdef DEBUG_EVALUATE
      MessageInterface::ShowMessage
         (wxT("   refObject=<%p><%p>'%s'\n"), refObject, refObject->GetTypeName().c_str(),
          refObject->GetName().c_str());
      #endif
      
      ElementWrapper *wrapper = FindWrapper(refObjectName);
      
      if (elementType == Gmat::REAL_TYPE || elementType == Gmat::RMATRIX_TYPE)
      {
         realValue = wrapper->EvaluateReal();
      }
      else
      {
         throw MathException
            (wxT("MathElement::Evaluate() Cannot Evaluate MathElementType of \"") +
             refObjectName + wxT("\""));
      }
      
      #ifdef DEBUG_EVALUATE
      MessageInterface::ShowMessage
         (wxT("MathElement::Evaluate() It's a parameter: %s realValue = %f\n"),
          refObject->GetName().c_str(), realValue);
      #endif
      
      return realValue;
   }
   else
   {
      #ifdef DEBUG_EVALUATE
      MessageInterface::ShowMessage
         (wxT("MathElement::Evaluate() It's a number: realValue = %f\n"), realValue);
      #endif
      
      return realValue;
   }
}


//------------------------------------------------------------------------------
// bool MatrixEvaluate()
//------------------------------------------------------------------------------
Rmatrix MathElement::MatrixEvaluate()
{
   #ifdef DEBUG_EVALUATE
   MessageInterface::ShowMessage
      (wxT("MathElement::MatrixEvaluate() this='%s', refObjectName='%s', refObject=<%p>, ")
       wxT("elementType=%d\n"), GetName().c_str(), refObjectName.c_str(), refObject, elementType);
   #endif
   
   // If this MathElement is function Input, just return since it is handled in
   // the FunctionRunner
   
   if (isFunctionInput)
      throw MathException(wxT("MathElement::MatrixEvaluate() Function input should ")
                          wxT("not be handled here"));
   
   if (elementType == Gmat::RMATRIX_TYPE)
   {
      if (refObject)
      {
         #ifdef DEBUG_EVALUATE
         Rmatrix rmat = refObject->GetRmatrix();
         MessageInterface::ShowMessage
            (wxT("MathElement::MatrixEvaluate() It's an Array: %s matVal =\n%s\n"),
             refObject->GetName().c_str(), rmat.ToString().c_str());
         #endif
         
         ElementWrapper *wrapper = FindWrapper(refObjectName);
         return wrapper->EvaluateArray();
      }
      else
      {
         #ifdef DEBUG_EVALUATE
         MessageInterface::ShowMessage
            (wxT("MathElement::MatrixEvaluate() It's a Rmatrix. matVal =\n%s\n"),
             matrix.ToString().c_str());
         #endif
         
         return matrix;
      }
   }
   else
   {
      Real rval = Evaluate();
      
      #ifdef DEBUG_EVALUATE
      MessageInterface::ShowMessage
         (wxT("MathElement::MatrixEvaluate() It's a number: rval = %f\n"), rval);
      #endif
      
      // Set matrix 1x1 and return
      Rmatrix rmat(1, 1, rval);
      return rmat;
      //throw MathException(wxT("MathElement::MatrixEvaluate() Invalid matrix"));
   }
}


//------------------------------------------------------------------------------
// bool SetChildren(MathNode *leftChild, MathNode *rightChild)
//------------------------------------------------------------------------------
bool MathElement::SetChildren(MathNode *leftChild, MathNode *rightChild)
{
   throw MathException(wxT("SetChildren() is not valid for MathElement"));
}


//------------------------------------------------------------------------------
// MathNode* GetLeft()
//------------------------------------------------------------------------------
MathNode* MathElement::GetLeft()
{
   return NULL;
}


//------------------------------------------------------------------------------
// MathNode* GetRight()
//------------------------------------------------------------------------------
MathNode* MathElement::GetRight()
{
   return NULL;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/*
 * Renames referenced objects
 *
 * @param <type> type of the reference object.
 * @param <oldName> old name of the reference object.
 * @param <newName> new name of the reference object.
 *
 * @return always true to indicate RenameRefObject() was implemented.
 */
//---------------------------------------------------------------------------
bool MathElement::RenameRefObject(const Gmat::ObjectType type,
                                  const wxString &oldName,
                                  const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("MathElement::RenameRefObject() refObjectName=%s, type=%d, oldName=<%s>, ")
       wxT("newName=<%s>\n"), refObjectName.c_str(), type, oldName.c_str(), newName.c_str());
   #endif
   
   if (refObjectName.find(oldName) != refObjectName.npos)
   {
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage(wxT("   old refObjectName=%s\n"), refObjectName.c_str());
      #endif
         
      refObjectName = GmatStringUtil::ReplaceName(refObjectName, oldName, newName);
         
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage(wxT("   new refObjectName=%s\n"), refObjectName.c_str());
      #endif
   }
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   now checking %d objects wrapperObjectNames\n"), wrapperObjectNames.size());
   #endif
   for (UnsignedInt i = 0; i < wrapperObjectNames.size(); i++)
   {
      wxString wrapperObjName = wrapperObjectNames[i];
      if (wrapperObjName.find(oldName) != wrapperObjName.npos)
      {
         wrapperObjName = GmatStringUtil::ReplaceName(wrapperObjName, oldName, newName);
         wrapperObjectNames[i] = wrapperObjName;
      }
   }
   
   if (theWrapperMap == NULL)
   {
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("MathElement::RenameRefObject() theWrapperMap is NULL, returning true\n"));
      #endif
      return true;
   }
   
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("   now checking %d objects in theWrapperMap\n"), theWrapperMap->size());
   #endif
   // Rename wrapper objects
   std::map<wxString, ElementWrapper *> tempMap;
   std::map<wxString, ElementWrapper *>::iterator ewi;
   bool needSwap = false;
   
   for (ewi = theWrapperMap->begin(); ewi != theWrapperMap->end(); ++ewi)
   {
      wxString name = ewi->first;
      
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage
         (wxT("   old name=<%s>, wrapper=%p\n"), (ewi->first).c_str(), ewi->second);
      #endif
      
      if (name == oldName)
      {
         name = newName;
         tempMap[name] = ewi->second;
         ewi->second->RenameObject(oldName, newName);
         needSwap = true;
      }
      else if (name.find(oldName) != name.npos)
      {
         name = GmatStringUtil::ReplaceName(name, oldName, newName);
         tempMap[name] = ewi->second;
         ewi->second->RenameObject(oldName, newName);
         needSwap = true;
      }
      else
      {
         // add other names into tempMap
         tempMap[name] = ewi->second;
      }
   }
   
   // If anything changed in the wrapper, swap the map
   if (needSwap)
      theWrapperMap->swap(tempMap);
   
   #ifdef DEBUG_RENAME
   for (ewi = theWrapperMap->begin(); ewi != theWrapperMap->end(); ++ewi)
      MessageInterface::ShowMessage
         (wxT("   new name=<%s>, wrapper=%p\n"), (ewi->first).c_str(), ewi->second);
   MessageInterface::ShowMessage
      (wxT("MathElement::RenameRefObject() refObjectName=%s\n"), refObjectName.c_str());
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the MathElement.
 *
 * @return clone of the MathElement.
 *
 */
//------------------------------------------------------------------------------
GmatBase* MathElement::Clone(void) const
{
   return (new MathElement(*this));
}

//---------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//---------------------------------------------------------------------------
/**
 * Returns the reference object pointer.
 *
 * @param type type of the reference object.
 * @param name name of the reference object.
 *
 * @return reference object pointer.
 */
 //---------------------------------------------------------------------------
GmatBase* MathElement::GetRefObject(const Gmat::ObjectType type, 
                                    const wxString &name)
{
   switch (type)
   {
      case Gmat::PARAMETER:
         return refObject;
         
      default:
         break;
   }

   return GmatBase::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type, 
//                   const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets the reference object pointer.
 *
 * @param type type of the reference object.
 * @param name name of the reference object.
 *
 * @return true if successful false otherwise.
 */
 //---------------------------------------------------------------------------
bool MathElement::SetRefObject(GmatBase *obj, const Gmat::ObjectType type, 
                               const wxString &name)
{
   switch (type)
   {
      case Gmat::PARAMETER:
      {
         SetWrapperObject(obj, name);
         return true;
      }
      default:
         break;
   }    
   
   return GmatBase::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
/**
 * This method returns a name of the referenced objects.
 *
 * @return the name of the object of the requested type.
 */
 //------------------------------------------------------------------------------
wxString MathElement::GetRefObjectName(const Gmat::ObjectType type) const
{
   switch (type)
   {
      case Gmat::PARAMETER:
         return refObjectName;
         
      default:
         break;
   }
   
   return GmatBase::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method sets the name of the referenced objects.
 *
 * @return true when referenced object name is set sucessfully otherwise false.
 */
 //------------------------------------------------------------------------------
bool MathElement::SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
{
   #ifdef DEBUG_MATH_ELEMENT
   MessageInterface::ShowMessage
      (wxT("MathElement::SetRefObjectName() name=%s\n"), name.c_str());
   #endif
   
   switch (type)
   {
      case Gmat::PARAMETER:
         refObjectName = name;
         SetWrapperObjectNames(name);
         return true;
         
      default:
         break;
   }
   
   return GmatBase::SetRefObjectName(type, name);
}


//------------------------------------------------------------------------------
//  const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns the names of the reference object.
 *
 * @param <type> reference object type.  Gmat::UnknownObject returns all of the
 *               ref objects.
 *
 * @return The names of the reference object.
 */
//------------------------------------------------------------------------------
const StringArray& MathElement::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathElement::GetRefObjectNameArray() '%s' entered, type=%d\n"),
       GetName().c_str(), type);
   MessageInterface::ShowMessage
      (wxT("There are %d wrapper object names:\n"), wrapperObjectNames.size());
   for (UnsignedInt i = 0; i < wrapperObjectNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   [%d] %s\n"), i, wrapperObjectNames[i].c_str());
   #endif
   
   if (type == Gmat::PARAMETER || Gmat::UNKNOWN_OBJECT)
   {      
      #ifdef DEBUG_WRAPPERS
      MessageInterface::ShowMessage
         (wxT("MathElement::GetRefObjectNameArray() returning %d wrapper object names:\n"),
          wrapperObjectNames.size());
      for (UnsignedInt i = 0; i < wrapperObjectNames.size(); i++)
         MessageInterface::ShowMessage(wxT("   [%d] %s\n"), i, wrapperObjectNames[i].c_str());
      #endif
      
      return wrapperObjectNames;
   }
   
   return GmatBase::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// void SetWrapperObjectNames(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Sets ElementWrapper object names. This will also set multiple input arguments
 * for GmatFunction.
 *
 * @param  name  The name or name list separated by comma
 */
//------------------------------------------------------------------------------
void MathElement::SetWrapperObjectNames(const wxString &name)
{
   wrapperObjectNames = GmatStringUtil::SeparateBy(refObjectName, wxT(","), true);
   
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathElement::SetWrapperObjectNames() wrapperObjectNames are:\n"));
   for (UnsignedInt i=0; i<wrapperObjectNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   '%s'\n"), wrapperObjectNames[i].c_str());
   #endif
}


//------------------------------------------------------------------------------
// void SetWrapperObject(GmatBase *obj, const wxString &name)
//------------------------------------------------------------------------------
/*
 * Sets ElementWrapper object. This will also set multiple input arguments
 * for GmatFunction.
 *
 * @param  obj  The object pointer
 */
//------------------------------------------------------------------------------
void MathElement::SetWrapperObject(GmatBase *obj, const wxString &name)
{
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathElement::SetWrapperObject() obj=<%p>, name='%s'\n"), obj, name.c_str());
   #endif
   
   refObject = (Parameter*) obj;
   refObjectType = refObject->GetTypeName().c_str();
   
   // go through wrapperObjectNames
   for (UnsignedInt i=0; i<wrapperObjectNames.size(); i++)
   {
      if (name == wrapperObjectNames[i])
      {
         #ifdef DEBUG_WRAPPERS
         MessageInterface::ShowMessage
            (wxT("   wrapperName = '%s'\n"), wrapperObjectNames[i].c_str());
         #endif
         
         // Handle array index
         Integer row, col;
         wxString newName;
         GmatStringUtil::GetArrayIndex(wrapperObjectNames[i], row, col, newName);
         
         // Check if name is the same
         if (newName != name)
            throw MathException
               (wxT("MathElement::SetRefObject() Cannot find parameter name:") + name);
         
         if (refObjectType == wxT("Array"))
         {
            Array *arr = (Array*)refObject;     
            elementType = Gmat::RMATRIX_TYPE;
            Integer theRowCount = arr->GetRowCount();
            Integer theColCount = arr->GetColCount();
            
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("MathElement::SetRefObject() elementType=%d, theRowCount=%d, ")
                wxT("theColCount=%d\n"), elementType, theRowCount, theColCount);
            #endif
            
            if (!matrix.IsSized())
               matrix.SetSize(theRowCount, theColCount);
            else
            {
               #ifdef DEBUG_WRAPPERS
               MessageInterface::ShowMessage
                  (wxT("MathElement::SetRefObject() matrix already sized. ")
                   wxT("matrix.size=%d, %d\n"), matrix.GetNumRows(),
                   matrix.GetNumColumns());
               #endif
            }
            
            matrix = arr->GetRmatrix(); // initial value
            
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("MathElement::SetRefObject() name=%s, matrix=\n%s\n"), name.c_str(),
                matrix.ToString().c_str());
            #endif
            
         }
         else if (refObject->GetReturnType() == Gmat::REAL_TYPE)
         {
            elementType = Gmat::REAL_TYPE;
            realValue = refObject->GetReal(); // initial value
            
            #ifdef DEBUG_WRAPPERS
            MessageInterface::ShowMessage
               (wxT("MathElement::SetRefObject() name=%s, elementType=%d, ")
                wxT("realValue=%f\n"), GetName().c_str(), elementType, realValue);
            #endif
         }
      }
   }
}


//------------------------------------------------------------------------------
// ElementWrapper* FindWrapper(const wxString &name)
//------------------------------------------------------------------------------
/*
 * Finds ElementWrapper pointer from the map by given name
 */
//------------------------------------------------------------------------------
ElementWrapper* MathElement::FindWrapper(const wxString &name)
{
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathElement::FindWrapper() node='%s', name='%s', refObject=<%p>, ")
       wxT("elementType=%d, theWrapperMap=<%p>\n"), GetName().c_str(), name.c_str(),
       refObject, elementType, theWrapperMap);
   #endif
   
   if (theWrapperMap == NULL)
      throw MathException(wxT("MathElement::FindWrapper() theWrapperMap is NULL"));
   
   #ifdef DEBUG_WRAPPERS
   std::map<wxString, ElementWrapper *>::iterator ewi;
   for (ewi = theWrapperMap->begin(); ewi != theWrapperMap->end(); ++ewi)
      MessageInterface::ShowMessage
         (wxT("   wrapperName='%s', wrapper=<%p>, wrapperType=%d, wrapperDesc='%s'\n"),
          (ewi->first).c_str(), ewi->second, (ewi->second)->GetWrapperType(),
          (ewi->second)->GetDescription().c_str());
   #endif
   
   if (theWrapperMap->find(name) == theWrapperMap->end())
      throw MathException(wxT("MathElement::FindWrapper() Cannot find \"") + name +
                          wxT("\" from theWrapperMap"));
   
   ElementWrapper *wrapper = (*theWrapperMap)[name];
   if (wrapper == NULL)
      throw MathException
         (wxT("MathElement::FindWrapper() The ElementWrapper of \"") + name + wxT("\" is NULL"));
   
   #ifdef DEBUG_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathElement::FindWrapper() returning wrapper <%p>\n"), wrapper);
   #endif
   
   return wrapper;
}

