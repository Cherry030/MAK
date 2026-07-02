#region --- License ---
/*********************************************************************************
** Copyright (c) 1992-2025 MAK Technologies, Inc
** All rights reserved.
*********************************************************************************/
#endregion
/* printer.tlt */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
using System;
using System.IO;
namespace makVrl
{
    public class HLAPrinter
    {
        private byte[] myBytes;
        private TextWriter myWriter;
 
        public HLAPrinter(TextWriter writer)
        {
            myWriter = writer;
            myBytes = new byte[1];
        }
 
        public virtual void WriteLine()
        {
            myWriter.WriteLine();
        }
 
        public virtual void Write(string str)
        {
            myWriter.Write(str);
        }
 
        public virtual void Write(int v)
        {
            myWriter.Write(v);
        }
 
        #region HLA datatypes
 
        public virtual void WriteHLAboolean(bool v)
        {
            myWriter.Write((v) ? "HLAtrue" : "HLAfalse");
        }
 
        public virtual void WriteHLAoctet(byte v)
        {
            myBytes[0] = v;
            myWriter.Write(BitConverter.ToString(myBytes));
        }
 
        public virtual void WriteHLAoctetPairLE(char v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAoctetPairBE(char v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAinteger16LE(short v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAinteger16BE(short v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAinteger32LE(int v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAinteger32BE(int v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAinteger64LE(long v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAinteger64BE(long v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAfloat32LE(float v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAfloat32BE(float v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAfloat64LE(double v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteHLAfloat64BE(double v)
        {
            myWriter.Write(v);
        }
 
        #endregion
 
        #region RPR datatypes
 
        public virtual void WriteRPRboolean(bool v)
        {
            myWriter.Write((v) ? "true" : "false");
        }
 
        public virtual void WriteRPRunsignedInteger8BE(byte v)
        {
            myWriter.Write((Int32)v);
        }
 
        public virtual void WriteRPRunsignedInteger16BE(ushort v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteRPRunsignedInteger32BE(uint v)
        {
            myWriter.Write(v);
        }
 
        public virtual void WriteRPRunsignedInteger64BE(ulong v)
        {
            myWriter.Write(v);
        }
 
        #endregion
 
        public virtual void Writebool(bool v)
        {
            WriteRPRboolean(v);
        }
 
        public virtual void Writebyte(byte v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writechar(char v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writestring(string v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writeshort(short v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writeushort(ushort v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writeint(int v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writeuint(uint v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writelong(long v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writeulong(ulong v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writefloat(float v)
        {
            myWriter.Write(v);
        }
 
        public virtual void Writedouble(double v)
        {
            myWriter.Write(v);
        }
 
        #region "Enumerations"

        #endregion
    }
}
