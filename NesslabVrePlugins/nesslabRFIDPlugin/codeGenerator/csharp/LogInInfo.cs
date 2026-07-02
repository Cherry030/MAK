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
 
	public class LogInInfoInteractionClass
	{
      #region "Parameters"

	  /*  */
	  private ushort mypublisherType; // unsigned short
	  public ushort publisherType {
			get { 
				return mypublisherType; 
			}
			set { mypublisherType = value; }
	  }

	  /*  */
	  private udeivceIDnsigned__short mydeviceID; // udeivceIDnsigned short
	  public udeivceIDnsigned__short deviceID {
			get { 
				return mydeviceID; 
			}
			set { mydeviceID = value; }
	  }

	  /*  */
	  private byte__array__10 myserviceNumber; // byte array 10
	  public byte__array__10 serviceNumber {
			get { 
				return myserviceNumber; 
			}
			set { myserviceNumber = value; }
	  }

	  /*  */
	  private byte__array__10 mylogInName; // byte array 10
	  public byte__array__10 logInName {
			get { 
				return mylogInName; 
			}
			set { mylogInName = value; }
	  }

	  /*  */
	  private byte__array myaffiliation; // byte array
	  public byte__array affiliation {
			get { 
				return myaffiliation; 
			}
			set { myaffiliation = value; }
	  }

      #endregion
 
      #region message handling functions
      public void parseMessage(UniversalHlaInteractionMessage sm)
      {
         MemoryStream stream;
         HLAReader reader;
         byte[] val;
 
         val = sm.getAttribute("publisherType");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            mypublisherType = reader.Readushort();
         }

         val = sm.getAttribute("deviceID");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            mydeviceID.deserialize(reader);
         }

         val = sm.getAttribute("serviceNumber");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            myserviceNumber.deserialize(reader);
         }

         val = sm.getAttribute("logInName");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            mylogInName.deserialize(reader);
         }

         val = sm.getAttribute("affiliation");
         if(val != null)
         {
            stream = new MemoryStream(val);
            reader = new HLAReader(stream);
            myaffiliation.deserialize(reader);
         }
         
      }
 
      public UniversalHlaInteractionMessage getMessage()
      {         
         UniversalHlaInteractionMessage sm = new UniversalHlaInteractionMessage();                           
         sm.objClass = "HLAinteractionRoot.LogInInfo"; // object class
 
         MemoryStream stream;
         HLAWriter writer;
 
         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         writer.Writeushort(mypublisherType);
         sm.setAttribute("publisherType", stream.ToArray());

         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         mydeviceID.serialize(writer);
         sm.setAttribute("deviceID", stream.ToArray());

         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         myserviceNumber.serialize(writer);
         sm.setAttribute("serviceNumber", stream.ToArray());

         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         mylogInName.serialize(writer);
         sm.setAttribute("logInName", stream.ToArray());

         stream = new MemoryStream();
         writer = new HLAWriter(stream);
         myaffiliation.serialize(writer);
         sm.setAttribute("affiliation", stream.ToArray());

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
         writer.Write("publisherType: ");
         writer.Writeushort(mypublisherType);
         writer.WriteLine();
         writer.Write("deviceID: ");
         mydeviceID.print(writer);
         writer.WriteLine();
         writer.Write("serviceNumber: ");
         myserviceNumber.print(writer);
         writer.WriteLine();
         writer.Write("logInName: ");
         mylogInName.print(writer);
         writer.WriteLine();
         writer.Write("affiliation: ");
         myaffiliation.print(writer);
         writer.WriteLine();         
     }
 
      #endregion
   }
}