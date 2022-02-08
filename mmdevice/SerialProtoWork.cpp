///////////////////////////////////////////////////////////////////////////////
// FILE:          SerialProtoWork.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   SerialProtoWork adapter.  Needs accompanying firmware
// COPYRIGHT:     University of California, San Francisco, 2008
// LICENSE:       LGPL
// 
// AUTHOR:        Nico Stuurman, nico@cmp.ucsf.edu 11/09/2008
//                automatic device detection by Karl Hoover
//
//

#include "SerialProtoWork.h"
#include "ModuleInterface.h"
#include <sstream>
#include <cstdio>

#ifdef WIN32
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
#endif
#include "FixSnprintf.h"

const char* g_DeviceNameSerialProtoWorkHub = "SerialProtoWork-Hub";
const char* g_DeviceNameSerialProtoWorkShutter = "SerialProtoWork-Shutter";


// Global info about the state of the SerialProtoWork.  This should be folded into a class
const int g_Min_MMVersion = 1;
const int g_Max_MMVersion = 2;
const char* g_versionProp = "Version";
const char* g_normalLogicString = "Normal";
const char* g_invertedLogicString = "Inverted";

const char* g_On = "On";
const char* g_Off = "Off";

// static lock
MMThreadLock CSerialProtoWorkHub::lock_;

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_DeviceNameSerialProtoWorkHub, MM::HubDevice, "Hub (required)");
   RegisterDevice(g_DeviceNameSerialProtoWorkShutter, MM::ShutterDevice, "Shutter");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   if (strcmp(deviceName, g_DeviceNameSerialProtoWorkHub) == 0)
   {
      return new CSerialProtoWorkHub;
   }
   else if (strcmp(deviceName, g_DeviceNameSerialProtoWorkShutter) == 0)
   {
      return new CSerialProtoWorkShutter;
   }

   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// CSerialProtoWorkHUb implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
CSerialProtoWorkHub::CSerialProtoWorkHub() :
   initialized_ (false),
   shutterState_ (0)
{
   portAvailable_ = false;
   invertedLogic_ = false;
   timedOutputActive_ = false;

   InitializeDefaultErrorMessages();

   SetErrorText(ERR_PORT_OPEN_FAILED, "Failed opening SerialProtoWork USB device");
   SetErrorText(ERR_BOARD_NOT_FOUND, "Did not find an SerialProtoWork board with the correct firmware.  Is the SerialProtoWork board connected to this serial port?");
   SetErrorText(ERR_NO_PORT_SET, "Hub Device not found.  The SerialProtoWork Hub device is needed to create this device");
   std::ostringstream errorText;
   errorText << "The firmware version on the SerialProtoWork is not compatible with this adapter.  Please use firmware version ";
   errorText <<  g_Min_MMVersion << " to " << g_Max_MMVersion;
   SetErrorText(ERR_VERSION_MISMATCH, errorText.str().c_str());

   CPropertyAction* pAct = new CPropertyAction(this, &CSerialProtoWorkHub::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

   pAct = new CPropertyAction(this, &CSerialProtoWorkHub::OnLogic);
   CreateProperty("Logic", g_invertedLogicString, MM::String, false, pAct, true);

   AddAllowedValue("Logic", g_invertedLogicString);
   AddAllowedValue("Logic", g_normalLogicString);
}

CSerialProtoWorkHub::~CSerialProtoWorkHub()
{
   Shutdown();
}

void CSerialProtoWorkHub::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_DeviceNameSerialProtoWorkHub);
}

bool CSerialProtoWorkHub::Busy()
{
   return false;
}

// private and expects caller to:
// 1. guard the port
// 2. purge the port
int CSerialProtoWorkHub::GetControllerVersion(int& version)
{
   int ret = DEVICE_OK;
   unsigned char command[1];
   command[0] = 30;
   version = 0;

   ret = WriteToComPort(port_.c_str(), (const unsigned char*) command, 1);
   if (ret != DEVICE_OK)
      return ret;

   std::string answer;
   ret = GetSerialAnswer(port_.c_str(), "\r\n", answer);
   if (ret != DEVICE_OK)
      return ret;

   if (answer != "MM-Ard")
      return ERR_BOARD_NOT_FOUND;

   // Check version number of the SerialProtoWork
   command[0] = 31;
   ret = WriteToComPort(port_.c_str(), (const unsigned char*) command, 1);
   if (ret != DEVICE_OK)
      return ret;

   std::string ans;
   ret = GetSerialAnswer(port_.c_str(), "\r\n", ans);
   if (ret != DEVICE_OK) {
         return ret;
   }
   std::istringstream is(ans);
   is >> version;

   return ret;

}

bool CSerialProtoWorkHub::SupportsDeviceDetection(void)
{
   return true;
}

MM::DeviceDetectionStatus CSerialProtoWorkHub::DetectDevice(void)
{
   if (initialized_)
      return MM::CanCommunicate;

   // all conditions must be satisfied...
   MM::DeviceDetectionStatus result = MM::Misconfigured;
   char answerTO[MM::MaxStrLength];
   
   try
   {
      std::string portLowerCase = port_;
      for( std::string::iterator its = portLowerCase.begin(); its != portLowerCase.end(); ++its)
      {
         *its = (char)tolower(*its);
      }
      if( 0< portLowerCase.length() &&  0 != portLowerCase.compare("undefined")  && 0 != portLowerCase.compare("unknown") )
      {
         result = MM::CanNotCommunicate;
         // record the default answer time out
         GetCoreCallback()->GetDeviceProperty(port_.c_str(), "AnswerTimeout", answerTO);

         // device specific default communication parameters
         // for SerialProtoWork Duemilanova
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), MM::g_Keyword_Handshaking, g_Off);
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), MM::g_Keyword_BaudRate, "57600" );
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), MM::g_Keyword_StopBits, "1");
         // SerialProtoWork timed out in GetControllerVersion even if AnswerTimeout  = 300 ms
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), "AnswerTimeout", "500.0");
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), "DelayBetweenCharsMs", "0");
         MM::Device* pS = GetCoreCallback()->GetDevice(this, port_.c_str());
         pS->Initialize();
         // The first second or so after opening the serial port, the SerialProtoWork is waiting for firmwareupgrades.  Simply sleep 2 seconds.
         CDeviceUtils::SleepMs(2000);
         MMThreadGuard myLock(lock_);
         PurgeComPort(port_.c_str());
         int v = 0;
         int ret = GetControllerVersion(v);
         // later, Initialize will explicitly check the version #
         if( DEVICE_OK != ret )
         {
            LogMessageCode(ret,true);
         }
         else
         {
            // to succeed must reach here....
            result = MM::CanCommunicate;
         }
         pS->Shutdown();
         // always restore the AnswerTimeout to the default
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), "AnswerTimeout", answerTO);

      }
   }
   catch(...)
   {
      LogMessage("Exception in DetectDevice!",false);
   }

   return result;
}


int CSerialProtoWorkHub::Initialize()
{
   // Name
   int ret = CreateProperty(MM::g_Keyword_Name, g_DeviceNameSerialProtoWorkHub, MM::String, true);
   if (DEVICE_OK != ret)
      return ret;

   // The first second or so after opening the serial port, the SerialProtoWork is waiting for firmwareupgrades.  Simply sleep 1 second.
   CDeviceUtils::SleepMs(2000);

   MMThreadGuard myLock(lock_);

   // Check that we have a controller:
   PurgeComPort(port_.c_str());
   ret = GetControllerVersion(version_);
   if( DEVICE_OK != ret)
      return ret;

   if (version_ < g_Min_MMVersion || version_ > g_Max_MMVersion)
      return ERR_VERSION_MISMATCH;

   CPropertyAction* pAct = new CPropertyAction(this, &CSerialProtoWorkHub::OnVersion);
   std::ostringstream sversion;
   sversion << version_;
   CreateProperty(g_versionProp, sversion.str().c_str(), MM::Integer, true, pAct);

   ret = UpdateStatus();
   if (ret != DEVICE_OK)
      return ret;

   // turn off verbose serial debug messages
   // GetCoreCallback()->SetDeviceProperty(port_.c_str(), "Verbose", "0");

   initialized_ = true;
   return DEVICE_OK;
}

int CSerialProtoWorkHub::DetectInstalledDevices()
{
   if (MM::CanCommunicate == DetectDevice()) 
   {
      std::vector<std::string> peripherals; 
      peripherals.clear();
      peripherals.push_back(g_DeviceNameSerialProtoWorkShutter);
      for (size_t i=0; i < peripherals.size(); i++) 
      {
         MM::Device* pDev = ::CreateDevice(peripherals[i].c_str());
         if (pDev) 
         {
            AddInstalledDevice(pDev);
         }
      }
   }

   return DEVICE_OK;
}



int CSerialProtoWorkHub::Shutdown()
{
   initialized_ = false;
   return DEVICE_OK;
}

int CSerialProtoWorkHub::OnPort(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
      pProp->Set(port_.c_str());
   }
   else if (pAct == MM::AfterSet)
   {
      pProp->Get(port_);
      portAvailable_ = true;
   }
   return DEVICE_OK;
}

int CSerialProtoWorkHub::OnVersion(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
      pProp->Set((long)version_);
   }
   return DEVICE_OK;
}

int CSerialProtoWorkHub::OnLogic(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
      if (invertedLogic_)
         pProp->Set(g_invertedLogicString);
      else
         pProp->Set(g_normalLogicString);
   } else if (pAct == MM::AfterSet)
   {
      std::string logic;
      pProp->Get(logic);
      if (logic.compare(g_invertedLogicString)==0)
         invertedLogic_ = true;
      else invertedLogic_ = false;
   }
   return DEVICE_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CSerialProtoWorkShutter implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

CSerialProtoWorkShutter::CSerialProtoWorkShutter() : initialized_(false), name_(g_DeviceNameSerialProtoWorkShutter)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   SetErrorText(ERR_NO_PORT_SET, "Hub Device not found.  The SerialProtoWork Hub device is needed to create this device");

   // Name
   int ret = CreateProperty(MM::g_Keyword_Name, g_DeviceNameSerialProtoWorkShutter, MM::String, true);
   assert(DEVICE_OK == ret);

   // Description
   ret = CreateProperty(MM::g_Keyword_Description, "SerialProtoWork shutter driver", MM::String, true);
   assert(DEVICE_OK == ret);

   // parent ID display
   CreateHubIDProperty();
}

CSerialProtoWorkShutter::~CSerialProtoWorkShutter()
{
   Shutdown();
}

void CSerialProtoWorkShutter::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_DeviceNameSerialProtoWorkShutter);
}

bool CSerialProtoWorkShutter::Busy()
{
   MM::MMTime interval = GetCurrentMMTime() - changedTime_;

   if (interval < (1000.0 * GetDelayMs() ))
      return true;
   else
       return false;
}

int CSerialProtoWorkShutter::Initialize()
{
   CSerialProtoWorkHub* hub = static_cast<CSerialProtoWorkHub*>(GetParentHub());
   if (!hub || !hub->IsPortAvailable()) {
      return ERR_NO_PORT_SET;
   }
   char hubLabel[MM::MaxStrLength];
   hub->GetLabel(hubLabel);
   SetParentID(hubLabel); // for backward comp.

   // set property list
   // -----------------
   
   // OnOff
   // ------
   CPropertyAction* pAct = new CPropertyAction (this, &CSerialProtoWorkShutter::OnOnOff);
   int ret = CreateProperty("OnOff", "0", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   // set shutter into the off state
   //WriteToPort(0);

   std::vector<std::string> vals;
   vals.push_back("0");
   vals.push_back("1");
   ret = SetAllowedValues("OnOff", vals);
   if (ret != DEVICE_OK)
      return ret;

   ret = UpdateStatus();
   if (ret != DEVICE_OK)
      return ret;

   changedTime_ = GetCurrentMMTime();
   initialized_ = true;

   return DEVICE_OK;
}

int CSerialProtoWorkShutter::Shutdown()
{
   if (initialized_)
   {
      initialized_ = false;
   }
   return DEVICE_OK;
}

int CSerialProtoWorkShutter::SetOpen(bool open)
{
	std::ostringstream os;
	os << "Request " << open;
	LogMessage(os.str().c_str(), true);

   if (open)
      return SetProperty("OnOff", "1");
   else
      return SetProperty("OnOff", "0");
}

int CSerialProtoWorkShutter::GetOpen(bool& open)
{
   char buf[MM::MaxStrLength];
   int ret = GetProperty("OnOff", buf);
   if (ret != DEVICE_OK)
      return ret;
   long pos = atol(buf);
   pos > 0 ? open = true : open = false;

   return DEVICE_OK;
}

int CSerialProtoWorkShutter::Fire(double /*deltaT*/)
{
   return DEVICE_UNSUPPORTED_COMMAND;
}

int CSerialProtoWorkShutter::WriteToPort(long value)
{
   CSerialProtoWorkHub* hub = static_cast<CSerialProtoWorkHub*>(GetParentHub());
   if (!hub || !hub->IsPortAvailable())
      return ERR_NO_PORT_SET;

   MMThreadGuard myLock(hub->GetLock());

   value = 63 & value;
   if (hub->IsLogicInverted())
      value = ~value;

   hub->PurgeComPortH();

   unsigned char command[2];
   command[0] = 1;
   command[1] = (unsigned char) value;
   int ret = hub->WriteToComPortH((const unsigned char*) command, 2);
   if (ret != DEVICE_OK)
      return ret;

   MM::MMTime startTime = GetCurrentMMTime();
   unsigned long bytesRead = 0;
   unsigned char answer[1];
   while ((bytesRead < 1) && ( (GetCurrentMMTime() - startTime).getMsec() < 250)) {
      ret = hub->ReadFromComPortH(answer, 1, bytesRead);
      if (ret != DEVICE_OK)
         return ret;
   }
   if (answer[0] != 1)
      return ERR_COMMUNICATION;

   hub->SetTimedOutput(false);

   return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////

int CSerialProtoWorkShutter::OnOnOff(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   CSerialProtoWorkHub* hub = static_cast<CSerialProtoWorkHub*>(GetParentHub());
   if (eAct == MM::BeforeGet)
   {
      // use cached state
      pProp->Set((long)hub->GetShutterState());
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);
      int ret;
      if (pos == 0)
         ret = WriteToPort(0); // turn everything off
      else
         ret = WriteToPort(hub->GetShutterState()); // restore old setting
      if (ret != DEVICE_OK)
         return ret;
      hub->SetShutterState(pos);
      changedTime_ = GetCurrentMMTime();
   }

   return DEVICE_OK;
}


