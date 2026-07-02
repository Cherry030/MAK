#region --- License ---
/*********************************************************************************
** Copyright (c) 1992-2025 MAK Technologies, Inc
** All rights reserved.
*********************************************************************************/
#endregion
/* talkIntr.tlt */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
using System;
using System.Collections.Generic;
using makVrl;
using System.Threading;
using System.Diagnostics;
 
namespace talk_LogInInfo
{
   class talk
   {
      static void Main(string[] args)
      {
 
         #region PID
 
         //Application set up. Get PID
         int pid = Process.GetCurrentProcess().Id;
         string pidStr = pid.ToString();
         string appName = "talk-LogInInfo-Sharp";
         appName += pid;
 
         #endregion
 
         //Create the application initializer. This handles initializing the of the
         //Exercise connection initializer and the protocol to instantiate
         ApplicationInitializer appInit = new ApplicationInitializer(args);
 
         List<String> cmdArgs = new List<String>(appInit.cmdArgs);
 
         //if we failed on command line parsing, quit
         if (appInit.connectionParseFail)
            return;
 
         //Default values
         if (appInit.exConnInit.connectionType == ExerciseConnection.ConnectionType.HLA13)
         {
            Hla13ExerciseConnectionInitializer hlaInit = (Hla13ExerciseConnectionInitializer)appInit.exConnInit;
            hlaInit.federationName = "generated";
            hlaInit.fedFilename = "C:/MAK/vrengage2.2/NesslabVrePlugins/dstNetworkPlugin/codeGenerator/SBE-Ext-FOM_v1.4.xml";
            hlaInit.enableGenericAttributes = true;
         }
         else if (appInit.exConnInit.connectionType == ExerciseConnection.ConnectionType.HLA1516)
         {
            Hla1516ExerciseConnectionInitializer hlaInit = (Hla1516ExerciseConnectionInitializer)appInit.exConnInit;
            hlaInit.federationName = "generated";
            hlaInit.fedFilename = "C:/MAK/vrengage2.2/NesslabVrePlugins/dstNetworkPlugin/codeGenerator/SBE-Ext-FOM_v1.4.xml";
            hlaInit.enableGenericAttributes = true;
         }
         else if (appInit.exConnInit.connectionType == ExerciseConnection.ConnectionType.HLA1516E)
         {
            Hla1516eExerciseConnectionInitializer hlaInit = (Hla1516eExerciseConnectionInitializer)appInit.exConnInit;
            hlaInit.federationName = "generated";
            hlaInit.fedFilename = "C:/MAK/vrengage2.2/NesslabVrePlugins/dstNetworkPlugin/codeGenerator/SBE-Ext-FOM_v1.4.xml";
            hlaInit.enableGenericAttributes = true;
         }
 
         if(appInit.parseCmdLine(cmdArgs) == 0)
            return;
 
         appInit.setApplicationId(pid);
 
         //Create ExerciseConnection with our initializer
         ExerciseConnection exConn = new ExerciseConnection(appInit.exConnInit);
 
         //Create our entity and its publisher
         LogInInfoInteractionClass obj = new LogInInfoInteractionClass();
         UniversalHlaInteractionMessage msg = obj.getMessage();
 
         Clock clock = exConn.clock;
 
         double dt = 0.05;
         double simTime = 0;
 
         Stopwatch stopwatch = new Stopwatch();
         stopwatch.Start();
 
         while (simTime <= 10.0)
         {
            Int64 startTime = stopwatch.ElapsedMilliseconds;
 
            // Tell VR-Link the current value of simulation time.
            clock.setSimTime(simTime);
 
            // Process any incoming messages.
            exConn.drainInput(-1.0);
 
            // Call tick, which insures that any data that needs to be
            // updated is sent.
            exConn.sendMessage(msg);
 
            simTime += dt;
            Int64 stopTime = stopwatch.ElapsedMilliseconds;
            Int64 duration = stopTime - startTime;
 
            //sleep for the duration of sim time, so that go wall-clock speed
            if (duration > 50)
               duration = 0;
            Thread.Sleep((Int32)(dt * 1000) - (Int32)duration);
         }
         exConn.Dispose();
      }
   }
}
