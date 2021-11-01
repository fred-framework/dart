// based on : https://cppsecrets.com/users/41129711010797106994610011511264103109971051084699111109/C00-Compare-version-string.php
#include <iostream>
#include <string>
#include <cstdio>

using namespace std;

// Define a structure to compare two version.
// This program is working upto four version-tags
//
// Let's say version is: 1.2.3.4
// major = 1
// minor = 2
// revision = 3
// build = 4
//
// Here, major, minor, revision and build are version-tags
//
// This program is working upto four version-tags.
// This code has been tested with various type of inputs including
// less than four version tags.
//
// LIMITATION: VERSION TAGS MUST BE SEPARATED BY DOT.
// 1.2.3.4 -- VALID INPUT VERSION
// 1-2-3-4 -- INVALID INPUT VERSION
// 1.2.3-4 -- INVALID INPUT VERSION

struct Version
{
   private:
   // Define four member variables major, minor, revision and build.
   // Here, we are saying it as version-tag
   int _major, _minor, _revision, _build;

   public:
   // Parametarized constructor. Pass string to it and it will
   // extract version-tag from it.
   //
   // Use initializer list to assign version-tag variables
   // Assign it by zero, otherwise std::scanf() will assign
   // garbage value to the versio-tag, if number of version-tag
   // will be less than four.
   Version(const std::string& version)
      : _major(0), _minor(0), _revision(0), _build(0)
   {
        assgin_from_string(version);
   }
   Version()
      : _major(0), _minor(0), _revision(0), _build(0)
   {   
       // do nothing
   }

   // Overload greater than(>) operator to compare two version objects
   bool operator > (const Version& other)
   {
      // Start comparing version tag from left most.
      // There are three relation between two number. It could be >, < or ==
      // While comparing the version tag, if they are equal, then move to the
      // next version-tag. If it could be greater then return true otherwise
      // return false.

      // Compare _major version-tag
      if (_major > other._major)
         return true;
      else if (_major < other._major)
         return false;

      // Compare moinor versio-tag
      // If control came here it means that above version-tag(s) are equal
      if (_minor > other._minor)
         return true;
      else if (_minor < other._minor)
         return false;

      // Compare _revision versio-tag
      // If control came here it means that above version-tag(s) are equal
      if (_revision > other._revision)
         return true;
      else if(_revision < other._revision)
         return false;

      // Compare _build versio-tag
      // If control came here it means that above version-tag(s) are equal
      if (_build > other._build)
         return true;
      else if(_build < other._build)
         return false;

      return false;
   }

   // Overload lower than(>) operator to compare two version objects
   bool operator < (const Version& other)
   {
       if (*this == other)
         return false;
       else if (*this > other)
          return false;
       return true;
   }


   // Overload equal to(==) operator to compare two version
   bool operator == (const Version& other)
   {
      return _major == other._major 
         && _minor == other._minor 
         && _revision == other._revision 
         && _build == other._build;
   }

   // Overload <<  operator to print the version
    friend std::ostream& operator<< (std::ostream& out, const Version& other)   
    //ostream & operator << (ostream &out, const Version &other)
    {
        out << other._major << "." <<  other._minor << "." <<  other._revision << "." <<  other._build;
        return out;
    }   

    // Overload the copy  operator= 
    Version& operator= (const string& other)
    {
        // do the copy
        assgin_from_string(other);
        return *this;
    }

 private:

    void assgin_from_string(string str){
        try{
            std::sscanf(str.c_str(), "%d.%d.%d.%d", &_major, &_minor, &_revision, &_build);
        }
        catch (std::system_error & e)
        {
            cerr << "Exception :: " << e.what() << endl;
            cerr << "ERROR: invalid version string " << str << endl;
            exit(EXIT_FAILURE);
        } 
      // version-tag must be >=0, if it is less than zero, then make it zero.
      if (_major < 0) _major = 0;
      if (_minor < 0) _minor = 0;
      if (_revision < 0) _revision = 0;
      if (_build < 0) _build = 0; 
    }   
};



/*
int main() {

   while(true) {
      std::string str_v1;
      std::string str_v2;

      cout << "Enter two versions." << endl;
      cin >> str_v1 >> str_v2;

      // Usage of version class
      Version v1(str_v1);
      Version v2(str_v2);

      if(v1 == v2) {
         cout << "Both the versions, " << str_v1 << " and " << str_v2 << " are equal" << endl;
      } else if(v1 > v2) {
         cout << "Version " << str_v1 << " is greater than version " << str_v2 << endl;
      } else {
         cout << "Version " << str_v1 << " is less than version " << str_v2 << endl;
      }
   }

   return 0;
}
*/