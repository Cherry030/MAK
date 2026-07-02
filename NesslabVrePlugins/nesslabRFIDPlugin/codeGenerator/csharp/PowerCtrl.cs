#region --- License ---
/*********************************************************************************
** Copyright (c) 1992-2025 MAK Technologies, Inc
** All rights reserved.
*********************************************************************************/
#endregion
/* interaction.tlt */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
using System;
using System.IO;
using System.Collections.Generic;
namespace makVrl
{
/*
 ..... 
 */
 
	public class PowerCtrlInteractionClass
	{
      #region "Parameters"

	  /* 1:Off, 2:Reboot */
	  private byte mypowerCtrlMsg; // byte
	  public byte powerCtrlMsg {
			get { 
				return mypowerCtrlMsg; 
			}
			set { mypowerCtrlMsg = value; }
	  }

	  /* 0:ALL, 1:DST, 2:CVT, 3:CCT */
	  private byte mydestinationSystem; // byte
	  public byte destinationSystem {
			get { 
				return mydestinationSystem; 
			}
			set { mydestinationSystem = value; }
	  }

      #endregion
 
      #region message handling functions
      public void parseMessage(UniversalHlaInteractionMessage sm)
      {
         MemoryStream stream;
         HLAReader reader;
         byte[] val;
 
         val = sm.getAttribute("powerCtrlMsg");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            mypowerCtrlMsg.deserialize(reader);
         }

         val = sm.getAttribute("destinationSystem");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            mydestinationSystem.deserialize(reader);
         }
         
      }
 
      public UniversalHlaInteractionMessage getMessage()
      {         
         UniversalHlaInteractionMessage sm = new UniversalHlaInteractionMessage();                           
         sm.objClass = "HLAinteractionRoot.PowerCtrl"; // object class
 
         MemoryStream stream;
         HLAWriter writer;
 
         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         mypowerCtrlMsg.serialize(writer);
         sm.setAttribute("powerCtrlMsg", stream.ToArray());

         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         mydestinationSystem.serialize(writer);
         sm.setAttribute("destinationSystem", stream.ToArray());

         return sm;
      }
 
      #endregion
 
      #region Printing
 
      public void print(HLAPrinter writer)
      {
         writer.Write("Interaction:");
         writer.WriteLine();
         writer.Write("--------");
         writer.WriteLine();
         printParameters(writer);
     }
 
      public void printParameters(HLAPrinter writer)
      {
         writer.Write("powerCtrlMsg: ");
         mypowerCtrlMsg.print(writer);
         writer.WriteLine();
         writer.Write("destinationSystem: ");
         mydestinationSystem.print(writer);
         writer.WriteLine();         
     }
 
      #endregion
   }
}