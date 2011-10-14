#ifndef _LIVE_P2PCOMMON2_BASE_PPL_UTIL_DETAIL_INI_FILE_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_UTIL_DETAIL_INI_FILE_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>

#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

using std::string;


namespace ppl { namespace util { namespace detail {

class CIniFile
{
public:
	struct Record
	{
		string Comments;
		char Commented;
		string Section;
		string Key;
		string Value;
	};

	enum CommentChar
	{
		Pound = '#',
		SemiColon = ';'
	};

	CIniFile(void);
	virtual ~CIniFile(void);

	static bool AddSection(string SectionName, string FileName);
	static bool CommentRecord(CommentChar cc, string KeyName,string SectionName,string FileName);
	static bool CommentSection(char CommentChar, string SectionName, string FileName);
	static string Content(string FileName);
	static bool Create(string FileName);
	static bool DeleteRecord(string KeyName, string SectionName, string FileName);
	static bool DeleteSection(string SectionName, string FileName);
	static vector<Record> GetRecord(string KeyName, string SectionName, string FileName);
	static vector<Record> GetSection(string SectionName, string FileName);
	static vector<string> GetSectionNames(string FileName);
	static string GetValue(string KeyName, string SectionName, string defaultValue, string FileName);
	static bool RecordExists(string KeyName, string SectionName, string FileName);
	static bool RenameSection(string OldSectionName, string NewSectionName, string FileName);
	static bool SectionExists(string SectionName, string FileName);
	static bool SetRecordComments(string Comments, string KeyName, string SectionName, string FileName);
	static bool SetSectionComments(string Comments, string SectionName, string FileName);
	static bool SetValue(string KeyName, string Value, string SectionName, string FileName);
	static bool Sort(string FileName, bool Descending);
	static bool UnCommentRecord(string KeyName,string SectionName,string FileName);
	static bool UnCommentSection(string SectionName, string FileName);

private:
	static vector<Record> GetSections(string FileName);
	static bool Load(string FileName, vector<Record>& content);	
	static bool Save(string FileName, vector<Record>& content);

	struct RecordSectionIs : std::unary_function<Record, bool>
	{
		std::string section_;

		RecordSectionIs(const std::string& section): section_(section){}

		bool operator()( const Record& rec ) const
		{
			return rec.Section == section_;
		}
	};

	struct RecordSectionKeyIs : std::unary_function<Record, bool>
	{
		std::string section_;
		std::string key_;

		RecordSectionKeyIs(const std::string& section, const std::string& key): section_(section),key_(key){}

		bool operator()( const Record& rec ) const
		{
			return ((rec.Section == section_)&&(rec.Key == key_));
		}
	};

	struct AscendingSectionSort
	{
		bool operator()(const Record& Start, const Record& End)
		{
			return Start.Section < End.Section;
		}
	};

	struct DescendingSectionSort
	{
		bool operator()(const Record& Start, const Record& End)
		{
			return Start.Section > End.Section;
		}
	};

	struct AscendingRecordSort
	{
		bool operator()(const Record& Start, const Record& End)
		{
			return Start.Key < End.Key;
		}
	};

	struct DescendingRecordSort
	{
		bool operator()(const Record& Start, const Record& End)
		{
			return Start.Key > End.Key;
		}
	};

	static void Trim(std::string& str, const std::string & ChrsToTrim = " \t\n\r", int TrimDir = 0);
};




inline CIniFile::CIniFile(void)													// Default constructor
{
}

inline CIniFile::~CIniFile(void)
{
}

// A function to trim whitespace from both sides of a given string
inline void CIniFile::Trim(std::string& str, const std::string & ChrsToTrim, int TrimDir)
{
	size_t startIndex = str.find_first_not_of(ChrsToTrim);
	if (startIndex == std::string::npos){str.erase(); return;}
	if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
	if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
}

//inline void TrimRight(std::string& str, const std::string & ChrsToTrim = " \t\n\r")
//{
//    Trim(str, ChrsToTrim, 2);
//}

//inline void TrimLeft(std::string& str, const std::string & ChrsToTrim = " \t\n\r")
//{
//    Trim(str, ChrsToTrim, 1);
//}

// A function to transform a string to uppercase if neccessary
//void UCase(string& str, bool ucase)
//{
//	if(ucase) transform(str.begin(), str.end(), str.begin(), toupper);
//}

inline bool CIniFile::Load(string FileName, vector<Record>& content)
{
	string s;																// Holds the current line from the ini file
	string CurrentSection;													// Holds the current section name

	std::ifstream inFile (FileName.c_str());										// Create an input filestream
	if (!inFile.is_open()) return false;									// If the input file doesn't open, then return
	content.clear();														// Clear the content vector

	string comments = "";													// A string to store comments in

	while(!std::getline(inFile, s).eof())									// Read until the end of the file
	{
		Trim(s);															// Trim whitespace from the ends
		if(!s.empty())														// Make sure its not a blank line
		{
			Record r;														// Define a new record

			if((s[0]=='#')||(s[0]==';'))									// Is this a commented line?
			{
				if ((s.find('[')==string::npos)&&							// If there is no [ or =
					(s.find('=')==string::npos))							// Then it's a comment
				{
					comments += s + '\n';									// Add the comment to the current comments string
				} else {
					r.Commented = s[0];										// Save the comment character
					s.erase(s.begin());										// Remove the comment for further processing
					Trim(s);
				}// Remove any more whitespace
			} else r.Commented = ' ';										// else mark it as not being a comment

			if(s.find('[')!=string::npos)									// Is this line a section?
			{		
				s.erase(s.begin());											// Erase the leading bracket
				s.erase(s.find(']'));										// Erase the trailing bracket
				r.Comments = comments;										// Add the comments string (if any)
				comments = "";												// Clear the comments for re-use
				r.Section = s;												// Set the Section value
				r.Key = "";													// Set the Key value
				r.Value = "";												// Set the Value value
				CurrentSection = s;
			}

			if(s.find('=')!=string::npos)									// Is this line a Key/Value?
			{
				r.Comments = comments;										// Add the comments string (if any)
				comments = "";												// Clear the comments for re-use
				r.Section = CurrentSection;									// Set the section to the current Section
				r.Key = s.substr(0,s.find('='));							// Set the Key value to everything before the = sign
				r.Value = s.substr(s.find('=')+1);							// Set the Value to everything after the = sign
			}
			if(comments == "")												// Don't add a record yet if its a comment line
				content.push_back(r);										// Add the record to content
		}
	}

	inFile.close();															// Close the file
	return true;
}

inline bool CIniFile::Save(string FileName, vector<Record>& content)
{
	std::ostringstream tmpName;
        tmpName << ((int)FileName.c_str() + (int)getpid());
	std::ofstream outFile (tmpName.str().c_str());							// Create an output filestream
	if (!outFile.is_open()) return false;									// If the output file doesn't open, then return

	for (int i=0;i<(int)content.size();i++)									// Loop through each vector
	{
		outFile << content[i].Comments;										// Write out the comments
		if(content[i].Key == "")											// Is this a section?
			outFile << content[i].Commented << "[" 
			<< content[i].Section << "]" << endl;							// Then format the section
		else
			outFile << content[i].Commented << content[i].Key  
			<< "=" << content[i].Value << endl;								// Else format a key/value
	}

	outFile.close();														// Close the file

	::remove(FileName.c_str());
	::rename(tmpName.str().c_str(), FileName.c_str());
	return true;
}

inline string CIniFile::Content(string FileName)
{
	string s="";															// Hold our return string
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file loads
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if(content[i].Comments != "") s += content[i].Comments;			// Add the comments
			if(content[i].Commented != ' ') s += content[i].Commented;		// If this is commented, then add it
			if((content[i].Key == ""))										// Is this a section?
				s += '[' + content[i].Section + ']';						// Add the section
			else s += content[i].Key + '=' + content[i].Value;				// Or the Key value to the return srting

			if (i !=(int)content.size()) s += '\n';								// If this is not the last line, add a CrLf
		}
		return s;															// Return the contents
	}

	return "";
}

inline std::vector<string> CIniFile::GetSectionNames(string FileName)
{
	std::vector<string> data;													// Holds the return data
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if(content[i].Key =="")											// If there is no key value, then its a section
				data.push_back(content[i].Section);							// Add the section to the return data
		}
	}

	return data;															// Return the data
}

inline std::vector<CIniFile::Record> CIniFile::GetSection(string SectionName, string FileName)
{
	vector<Record> data;													// Holds the return data
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if((content[i].Section == SectionName) &&						// If this is the section name we want
				(content[i].Key != ""))										// but not the section name itself
				data.push_back(content[i]);									// Add the record to the return data
		}
	}

	return data;															// Return the data
}

inline bool CIniFile::RecordExists(string KeyName, string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		std::vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found
	}
	return true;															// The Section/Key was found
}

inline bool CIniFile::SectionExists(string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionIs(SectionName));					// Locate the Section

		if (iter == content.end()) return false;							// The Section was not found
	}
	return true;															// The Section was found
}

inline std::vector<CIniFile::Record> CIniFile::GetRecord(string KeyName, string SectionName, string FileName)
{
	std::vector<Record> data;													// Holds the return data
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		std::vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Record

		if (iter == content.end()) return data;								// The Record was not found

		data.push_back (*iter);												// The Record was found
	}
	return data;															// Return the Record
}

inline string CIniFile::GetValue(string KeyName, string SectionName, string defaultValue, string FileName)
{
	std::vector<Record> content = GetRecord(KeyName,SectionName, FileName);		// Get the Record

	if(!content.empty())													// Make sure there is a value to return
		return content[0].Value;											// And return the value

	return defaultValue;																// No value was found
}

inline bool CIniFile::SetValue(string KeyName, string Value, string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		if(!SectionExists(SectionName,FileName))							// If the Section doesn't exist
		{
			Record s = {"",' ',SectionName,"",""};							// Define a new section
			Record r = {"",' ',SectionName,KeyName,Value};					// Define a new record
			content.push_back(s);											// Add the section
			content.push_back(r);											// Add the record
			return Save(FileName,content);									// Save
		}

		if(!RecordExists(KeyName,SectionName,FileName))						// If the Key doesn't exist
		{
			vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionIs(SectionName));					// Locate the Section
			iter++;															// Advance just past the section
			Record r = {"",' ',SectionName,KeyName,Value};						// Define a new record
			content.insert(iter,r);											// Add the record
			return Save(FileName,content);									// Save
		}

		vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Record

		iter->Value = Value;												// Insert the correct value
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

inline bool CIniFile::RenameSection(string OldSectionName, string NewSectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(vector<Record>::iterator iter = content.begin(); 
			iter < content.end(); iter++)									// Loop through the records
		{
			if(iter->Section == OldSectionName)								// Is this the OldSectionName?
				iter->Section = NewSectionName;								// Now its the NewSectionName
		}
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

inline bool CIniFile::CommentRecord(CommentChar cc, string KeyName,string SectionName,string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		std::vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found

		iter->Commented = cc;										// Change the Comment value
		return Save(FileName,content);										// Save

	}
	return false;															// In the event the file does not load
}

inline bool CIniFile::UnCommentRecord(string KeyName,string SectionName,string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		std::vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found

		iter->Commented = ' ';												// Remove the Comment value
		return Save(FileName,content);										// Save

	}
	return false;															// In the event the file does not load
}

inline bool CIniFile::CommentSection(char CommentChar, string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(std::vector<Record>::iterator iter = content.begin(); iter < content.end(); iter++)
		{
			if(iter->Section == SectionName)								// Is this the right section?
				iter->Commented = CommentChar;								// Change the comment value
		}
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

inline bool CIniFile::UnCommentSection(string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(std::vector<Record>::iterator iter = content.begin(); iter < content.end(); iter++)
		{
			if(iter->Section == SectionName)								// Is this the right section?
				iter->Commented = ' ';										// Remove the comment value
		}																	
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

inline bool CIniFile::DeleteRecord(string KeyName, string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		std::vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found

		content.erase(iter);												// Remove the Record
		return Save(FileName,content);										// Save

	}

	return false;															// In the event the file does not load
}

inline bool CIniFile::DeleteSection(string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(int i=(int)content.size()-1;i>-1;i--)								// Iterate backwards through the content
		{							
			if(content[i].Section == SectionName)							// Is this related to the Section?
				content.erase (content.begin()+i);							// Then erase it
		}

		return Save(FileName,content);										// Save
	}
	return false;															// In the event the file does not load
}

inline bool CIniFile::SetSectionComments(string Comments, string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(std::vector<Record>::iterator iter = content.begin(); iter < content.end(); iter++)									// Loop through the records
		{
			if((iter->Section == SectionName) &&							// Is this the Section?
				(iter->Key == ""))											// And not a record
			{	
				if (Comments.size() >= 2)									// Is there a comment?
				{
					if (Comments.substr(Comments.size()-2) != "\n")		// Does the string end in a newline?
						Comments += "\n";								// If not, add one
				}
				iter->Comments = Comments;								// Set the comments

				return Save(FileName,content);							// Save
			}
		}
	}
	return false;															// In the event the file does not load
}

inline bool CIniFile::SetRecordComments(string Comments, string KeyName, string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		std::vector<Record>::iterator iter = std::find_if(content.begin(), 
			content.end(), 
			CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found

		if (Comments.size() >= 2)											// Is there a comment?
		{
			if (Comments.substr(Comments.size()-2) != "\n")					// Does the string end in a newline?
				Comments += "\n";											// If not, add one
		}
		iter->Comments = Comments;											// Set the comments
		return Save(FileName,content);										// Save

	}

	return false;															// In the event the file does not load
}

inline vector<CIniFile::Record> CIniFile::GetSections(string FileName)
{
	std::vector<Record> data;													// Holds the return data
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if(content[i].Key == "")										// If this is a section 
				data.push_back(content[i]);									// Add the record to the return data
		}
	}

	return data;															// Return the data
}

inline bool CIniFile::Sort(string FileName, bool Descending)
{
	std::vector<CIniFile::Record> content;										// Used to hold the sorted content
	std::vector<CIniFile::Record> sections = GetSections(FileName);				// Get a list of Sections

	if(!sections.empty())													// Is there anything to process?
	{

		if(Descending)														// Descending or Ascending?
			std::sort(sections.begin(), sections.end(), DescendingSectionSort());
		else																// Sort the Sections
			std::sort(sections.begin(), sections.end(), AscendingSectionSort());

		for(std::vector<Record>::iterator iter = sections.begin(); iter < sections.end(); iter++) // For each Section
		{																		
			content.push_back(*iter);										// Add the sorted Section to the content

			std::vector<CIniFile::Record> records = GetSection(iter->Section ,FileName); // Get a list of Records for this section

			if(Descending)													// Descending or Ascending?
				std::sort(records.begin(), records.end(), DescendingRecordSort());
			else															// Sort the Records
				std::sort(records.begin(), records.end(), AscendingRecordSort());

			for(std::vector<Record>::iterator it = records.begin(); it < records.end(); it++) // For each Record
				content.push_back(*it);										// Add the sorted Record to the content
		}

		return Save(FileName,content);										// Save
	}

	return false;															// There were no sections
}

inline bool CIniFile::AddSection(string SectionName, string FileName)
{
	std::vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		Record s = {"",' ',SectionName,"",""};								// Define a new section
		content.push_back(s);												// Add the section
		return Save(FileName,content);										// Save
	}

	return false;															// The file did not open
}

inline bool CIniFile::Create(string FileName)
{
	std::vector<Record> content;													// Create empty content
	return Save(FileName,content);											// Save
}

} } }

#endif
