//$Id: EditorPreferences.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              EditorPreferences
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2009/01/15
//
/**
 * Implements EditorPreferences.
 */
//------------------------------------------------------------------------------

#include "EditorPreferences.hpp"

//---------------------------------
// static data
//---------------------------------

//----------------------------------------------------------------------------
//! language types
const GmatEditor::CommonInfoType GmatEditor::globalCommonPrefs =
{
    // editor functionality prefs
    true,  // syntaxEnable
    true,  // foldEnable
    true,  // indentEnable
    // display defaults prefs
    false, // overTypeInitial
    false, // readOnlyInitial
    false, // wrapModeInitial
    false, // displayEOLEnable
    false, // IndentGuideEnable
    true,  // lineNumberEnable
    false, // longLineOnEnable
    false, // whiteSpaceEnable
};

//----------------------------------------------------------------------------
// keywordlists
// GMAT
wxChar* GmatCommands =
   wxT("GMAT Create Global Maneuver Propagate Report Save Stop Toggle ")
   wxT("Achieve Vary Target Optimize Minimize PenDown PenUp ")
   wxT("For EndFor If Else EndIf While EndWhile Target EndTarget ")
   wxT("BeginFiniteBurn EndFiniteBurn BeginScript EndScript " )
   wxT("Spacecraft ForceModel Propagator FuelTank Thruster SolarSystem ")
   wxT("CoordinateSystem Variable Array String ReportFile XYPlot OpenGLPlot " )
   wxT("ImpulsiveBurn FiniteBurn DifferentialCorrector Optimizer MatlabFunction" );
wxChar* GmatObjectTypes =
   wxT("Spacecraft ForceModel Propagator FuelTank Thruster SolarSystem ")
   wxT("CoordinateSystem Variable Array String ReportFile XYPlot OpenGLPlot " )
   wxT("ImpulsiveBurn FiniteBurn DifferentialCorrector Optimizer MatlabFunction" );
wxChar* GmatComments =
   wxT("%");
// C++
wxChar* CppWordlist1 =
   wxT("asm auto bool break case catch char class const const_cast ")
   wxT("continue default delete do double dynamic_cast else enum explicit ")
   wxT("export extern false float for friend goto if inline int long ")
   wxT("mutable namespace new operator private protected public register ")
   wxT("reinterpret_cast return short signed sizeof static static_cast ")
   wxT("struct switch template this throw true try typedef typeid ")
   wxT("typename union unsigned using virtual void volatile wchar_t ")
   wxT("while");
wxChar* CppWordlist2 =
   wxT("file");
wxChar* CppWordlist3 =
   wxT("a addindex addtogroup anchor arg attention author b brief bug c ")
   wxT("class code date def defgroup deprecated dontinclude e em endcode ")
   wxT("endhtmlonly endif endlatexonly endlink endverbatim enum example ")
   wxT("exception f$ f[ f] file fn hideinitializer htmlinclude ")
   wxT("htmlonly if image include ingroup internal invariant interface ")
   wxT("latexonly li line link mainpage name namespace nosubgrouping note ")
   wxT("overload p page par param post pre ref relates remarks return ")
   wxT("retval sa section see showinitializer since skip skipline struct ")
   wxT("subsection test throw todo typedef union until var verbatim ")
   wxT("verbinclude version warning weakgroup $ @ \"\" & < > # { }");

// Python
wxChar* PythonWordlist1 =
   wxT("and assert break class continue def del elif else except exec ")
   wxT("finally for from global if import in is lambda None not or pass ")
   wxT("print raise return try while yield");
wxChar* PythonWordlist2 =
   wxT("ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN ")
   wxT("BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX CLASS ")
   wxT("COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX ")
   wxT("DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX ICON LANGUAGE ")
   wxT("LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON ")
   wxT("RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 ")
   wxT("STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY");


//----------------------------------------------------------------------------
//! languages
//------------------------------------------------
// Lexical states for SCLEX_MATLAB (from stc.h)
//------------------------------------------------
//#define wxSTC_MATLAB_DEFAULT 0
//#define wxSTC_MATLAB_COMMENT 1
//#define wxSTC_MATLAB_COMMAND 2
//#define wxSTC_MATLAB_NUMBER  3
//#define wxSTC_MATLAB_KEYWORD 4
// single quoted string
//#define wxSTC_MATLAB_STRING 5
//#define wxSTC_MATLAB_OPERATOR 6
//#define wxSTC_MATLAB_IDENTIFIER 7
//#define wxSTC_MATLAB_DOUBLEQUOTESTRING 8
//------------------------------------------------

const GmatEditor::LanguageInfoType GmatEditor::globalLanguagePrefs [] = {
   //-------------------------------------------------------
   // GMAT script, function, Matlab scripts
   //-------------------------------------------------------
   {wxT("GMAT"),
    wxT("*.script;*.m;*.gmf"),
    #if 0
    // matlab style
    wxSTC_LEX_MATLAB, // Shows GMAT comments, but no commands
    {{GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_COMMENT_LINE, NULL},
     {GMAT_STC_TYPE_COMMENT_DOC, NULL},
     {GMAT_STC_TYPE_NUMBER, NULL},
     {GMAT_STC_TYPE_WORD1, GmatCommands},   // KEYWORDS
     {GMAT_STC_TYPE_CHARACTER, NULL},
     {GMAT_STC_TYPE_OPERATOR, NULL},
     {GMAT_STC_TYPE_IDENTIFIER, NULL},
     {GMAT_STC_TYPE_STRING, NULL},
     {GMAT_STC_TYPE_STRING_EOL, NULL},
     {GMAT_STC_TYPE_UUID, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},         // VERBATIM
     {GMAT_STC_TYPE_REGEX, NULL},
     {GMAT_STC_TYPE_COMMENT_SPECIAL, NULL}, // DOXY
     {GMAT_STC_TYPE_WORD2, GmatObjectTypes},// EXTRA WORDS
     {GMAT_STC_TYPE_WORD3, GmatCommands},   // DOXY KEYWORDS
     {GMAT_STC_TYPE_ERROR, NULL},           // KEYWORDS ERROR
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL}},
    GMAT_STC_FOLD_COMMENT | GMAT_STC_FOLD_COMPACT |
    GMAT_STC_FOLD_PREPROC},
   #endif
    // python style
    #if 1
    wxSTC_LEX_PYTHON, // Shows GMAT commands and folds, but no comments, no object types
    {{GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_COMMENT_LINE, NULL},
     {GMAT_STC_TYPE_NUMBER, NULL},
     {GMAT_STC_TYPE_STRING, NULL},
     {GMAT_STC_TYPE_CHARACTER, NULL},
     {GMAT_STC_TYPE_WORD1, GmatCommands},      // KEYWORDS
     {GMAT_STC_TYPE_DEFAULT, NULL},            // TRIPLE
     {GMAT_STC_TYPE_DEFAULT, NULL},            // TRIPLEDOUBLE
     {GMAT_STC_TYPE_DEFAULT, NULL},            // CLASSNAME
     {GMAT_STC_TYPE_DEFAULT, GmatObjectTypes}, // DEFNAME
     {GMAT_STC_TYPE_OPERATOR, NULL},
     {GMAT_STC_TYPE_IDENTIFIER, NULL},
     {GMAT_STC_TYPE_DEFAULT, GmatComments},    // COMMENT_BLOCK
     {GMAT_STC_TYPE_STRING_EOL, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL}},
    GMAT_STC_FOLD_COMMENT | GMAT_STC_FOLD_COMPACT |
    GMAT_STC_FOLD_PREPROC},
   #endif
    // cpp style
    #if 0
    wxSTC_LEX_CPP, // Shows GMAT commands, object types, but no comments, no folds
    {{GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_COMMENT, NULL},
     {GMAT_STC_TYPE_COMMENT_LINE, NULL},
     {GMAT_STC_TYPE_COMMENT_DOC, NULL},
     {GMAT_STC_TYPE_NUMBER, NULL},
     {GMAT_STC_TYPE_WORD1, GmatCommands},   // KEYWORDS
     {GMAT_STC_TYPE_STRING, NULL},
     {GMAT_STC_TYPE_CHARACTER, NULL},
     {GMAT_STC_TYPE_UUID, NULL},
     {GMAT_STC_TYPE_PREPROCESSOR, NULL},
     {GMAT_STC_TYPE_OPERATOR, NULL},
     {GMAT_STC_TYPE_IDENTIFIER, NULL},
     {GMAT_STC_TYPE_STRING_EOL, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},         // VERBATIM
     {GMAT_STC_TYPE_REGEX, NULL},
     {GMAT_STC_TYPE_COMMENT_SPECIAL, NULL}, // DOXY
     {GMAT_STC_TYPE_WORD2, GmatObjectTypes},// EXTRA WORDS
     {GMAT_STC_TYPE_WORD3, GmatCommands},   // DOXY KEYWORDS
     {GMAT_STC_TYPE_ERROR, NULL},           // KEYWORDS ERROR
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL}},
    GMAT_STC_FOLD_COMMENT | GMAT_STC_FOLD_COMPACT |
    GMAT_STC_FOLD_PREPROC},
    #endif
   // C++
   {wxT("C++"),
    wxT("*.c;*.cc;*.cpp;*.cxx;*.cs;*.h;*.hh;*.hpp;*.hxx;*.sma"),
    wxSTC_LEX_CPP,
    {{GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_COMMENT, NULL},
     {GMAT_STC_TYPE_COMMENT_LINE, NULL},
     {GMAT_STC_TYPE_COMMENT_DOC, NULL},
     {GMAT_STC_TYPE_NUMBER, NULL},
     {GMAT_STC_TYPE_WORD1, CppWordlist1},   // KEYWORDS
     {GMAT_STC_TYPE_STRING, NULL},
     {GMAT_STC_TYPE_CHARACTER, NULL},
     {GMAT_STC_TYPE_UUID, NULL},
     {GMAT_STC_TYPE_PREPROCESSOR, NULL},
     {GMAT_STC_TYPE_OPERATOR, NULL},
     {GMAT_STC_TYPE_IDENTIFIER, NULL},
     {GMAT_STC_TYPE_STRING_EOL, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},         // VERBATIM
     {GMAT_STC_TYPE_REGEX, NULL},
     {GMAT_STC_TYPE_COMMENT_SPECIAL, NULL}, // DOXY
     {GMAT_STC_TYPE_WORD2, CppWordlist2},   // EXTRA WORDS
     {GMAT_STC_TYPE_WORD3, CppWordlist3},   // DOXY KEYWORDS
     {GMAT_STC_TYPE_ERROR, NULL},           // KEYWORDS ERROR
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL}},
    GMAT_STC_FOLD_COMMENT | GMAT_STC_FOLD_COMPACT |
    GMAT_STC_FOLD_PREPROC},
   // Python
   {wxT("Python"),
    wxT("*.py;*.pyw"),
    wxSTC_LEX_PYTHON,
    {{GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_COMMENT_LINE, NULL},
     {GMAT_STC_TYPE_NUMBER, NULL},
     {GMAT_STC_TYPE_STRING, NULL},
     {GMAT_STC_TYPE_CHARACTER, NULL},
     {GMAT_STC_TYPE_WORD1, PythonWordlist1},   // KEYWORDS
     {GMAT_STC_TYPE_DEFAULT, NULL},            // TRIPLE
     {GMAT_STC_TYPE_DEFAULT, NULL},            // TRIPLEDOUBLE
     {GMAT_STC_TYPE_DEFAULT, NULL},            // CLASSNAME
     {GMAT_STC_TYPE_DEFAULT, PythonWordlist2}, // DEFNAME
     {GMAT_STC_TYPE_OPERATOR, NULL},
     {GMAT_STC_TYPE_IDENTIFIER, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},            // COMMENT_BLOCK
     {GMAT_STC_TYPE_STRING_EOL, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL}},
    GMAT_STC_FOLD_COMMENTPY | GMAT_STC_FOLD_QUOTESPY},
   // * (any)
   {(wxChar *)DEFAULT_LANGUAGE,
    wxT("*.*"),
    wxSTC_LEX_PROPERTIES,
    {{GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},
     {GMAT_STC_TYPE_DEFAULT, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL},
     {-1, NULL}},
    0},
};

const int GmatEditor::globalLanguagePrefsSize = WXSIZEOF(GmatEditor::globalLanguagePrefs);

//----------------------------------------------------------------------------
//! style types
const GmatEditor::StyleInfoType GmatEditor::globalStylePrefs [] = {
   // GMAT_STC_TYPE_DEFAULT
   {wxT("Default"),
    wxT("BLACK"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_WORD1 (with wxSTC_LEX_PYTHON, GMAT KeyWords shows in this color)
   {wxT("Keyword1"),
    wxT("BLUE"), wxT("WHITE"),
    //wxT(""), 10, GMAT_STC_STYLE_BOLD, 0},
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_WORD2
   {wxT("Keyword2"),
    wxT("RED"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_WORD3
   {wxT("Keyword3"),
    wxT("CORNFLOWER BLUE"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_WORD4
   {wxT("Keyword4"),
    wxT("CYAN"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_WORD5
   {wxT("Keyword5"),
    wxT("DARK GREY"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_WORD6
   {wxT("Keyword6"),
    wxT("GREY"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_COMMENT
   {wxT("Comment"),
    wxT("FOREST GREEN"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_COMMENT_DOC
   {wxT("Comment (Doc)"),
    wxT("FOREST GREEN"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_COMMENT_LINE
   {wxT("Comment line"),
    wxT("FOREST GREEN"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_COMMENT_SPECIAL
   {wxT("Special comment"),
    wxT("FOREST GREEN"), wxT("WHITE"),
    wxT(""), 10, GMAT_STC_STYLE_ITALIC, 0},

   // GMAT_STC_TYPE_CHARACTER (with wxSTC_LEX_PYTHON, string inside single quote)
   {wxT("Character"),
    wxT("PURPLE"), wxT("WHITE"),
    //wxT("KHAKI"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_CHARACTER_EOL
   {wxT("Character (EOL)"),
    wxT("PURPLE"), wxT("WHITE"),
    //wxT("KHAKI"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_STRING (with wxSTC_LEX_PYTHON, string inside double quote)
   {wxT("String"),
    wxT("BROWN"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_STRING_EOL
   {wxT("String (EOL)"),
    wxT("BROWN"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_DELIMITER
   {wxT("Delimiter"),
    wxT("ORANGE"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_PUNCTUATION
   {wxT("Punctuation"),
    wxT("ORANGE"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_OPERATOR (with wxSTC_LEX_PYTHON, () [] math operators shown in this color)
   {wxT("Operator"),
    wxT("BLACK"), wxT("WHITE"),
    //wxT(""), 10, GMAT_STC_STYLE_BOLD, 0},
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_BRACE
   {wxT("Label"),
    wxT("VIOLET"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_COMMAND
   {wxT("Command"),
    wxT("BLUE"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_IDENTIFIER (with wxSTC_LEX_PYTHON, statesments showns in this color)
   {wxT("Identifier"),
    wxT("BLACK"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_LABEL
   {wxT("Label"),
    wxT("VIOLET"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_NUMBER
   {wxT("Number"),
    wxT("SIENNA"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_PARAMETER
   {wxT("Parameter"),
    wxT("VIOLET"), wxT("WHITE"),
    wxT(""), 10, GMAT_STC_STYLE_ITALIC, 0},

   // GMAT_STC_TYPE_REGEX
   {wxT("Regular expression"),
    wxT("ORCHID"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_UUID
   {wxT("UUID"),
    wxT("ORCHID"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_VALUE
   {wxT("Value"),
    wxT("ORCHID"), wxT("WHITE"),
    wxT(""), 10, GMAT_STC_STYLE_ITALIC, 0},

   // GMAT_STC_TYPE_PREPROCESSOR
   {wxT("Preprocessor"),
    wxT("GREY"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_SCRIPT
   {wxT("Script"),
    wxT("DARK GREY"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_ERROR
   {wxT("Error"),
    wxT("RED"), wxT("WHITE"),
    wxT(""), 10, 0, 0},

   // GMAT_STC_TYPE_UNDEFINED
   {wxT("Undefined"),
    wxT("ORANGE"), wxT("WHITE"),
    wxT(""), 10, 0, 0}

};

const int GmatEditor::globalStylePrefsSize = WXSIZEOF(GmatEditor::globalStylePrefs);
