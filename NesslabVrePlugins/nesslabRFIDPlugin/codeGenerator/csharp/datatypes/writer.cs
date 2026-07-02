#region --- License ---
/*********************************************************************************
** Copyright (c) 1992-2025 MAK Technologies, Inc
** All rights reserved.
*********************************************************************************/
#endregion
/* writer.tlt */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
using System;
using System.IO;
namespace makVrl
{
    public class HLAWriter : BinaryWriter
    {
        public HLAWriter(Stream outstream)
            : base(outstream, System.Text.Encoding.Unicode)
        {
 
        }
 
        // Skip implicit padding bytes
        public virtual void align(int boundary)
        {
            int mis = (int)(BaseStream.Position % (long)boundary);
            if (mis > 0)
            {
                WritePadding(boundary - mis);
            }
        }
 
        public virtual void WritePadding(int len)
        {
            Write(new byte[len]);
        }
 
        #region HLA datatypes
 
        public virtual void WriteHLAboolean(bool v)
        {
            WriteHLAinteger32BE((v) ? 1 : 0);
        }
 
        public virtual void WriteHLAoctet(byte v)
        {
            Write(v);
        }
 
        public virtual void WriteHLAoctetPairLE(char v)
        {
            align(2);
            Write(v);
        }
 
        public virtual void WriteHLAoctetPairBE(char v)
        {
            align(2);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteHLAinteger16LE(short v)
        {
            align(2);
            Write(v);
        }
 
        public virtual void WriteHLAinteger16BE(short v)
        {
            align(2);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteHLAinteger32LE(int v)
        {
            align(4);
            Write(v);
        }
 
        public virtual void WriteHLAinteger32BE(int v)
        {
            align(4);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteHLAinteger64LE(long v)
        {
            align(8);
            Write(v);
        }
 
        public virtual void WriteHLAinteger64BE(long v)
        {
            align(8);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteHLAfloat32LE(float v)
        {
            align(4);
            Write(v);
        }
 
        public virtual void WriteHLAfloat32BE(float v)
        {
            align(4);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteHLAfloat64LE(double v)
        {
            align(8);
            Write(v);
        }
 
        public virtual void WriteHLAfloat64BE(double v)
        {
            align(8);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        #endregion
 
        #region RPR datatypes
 
        public virtual void WriteRPRboolean(bool v)
        {
            Write((byte)((v) ? 1 : 0));
        }
 
        public virtual void WriteRPRunsignedInteger8BE(byte v)
        {
            Write(v);
        }
 
        public virtual void WriteRPRunsignedInteger16BE(ushort v)
        {
            align(2);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteRPRunsignedInteger32BE(uint v)
        {
            align(4);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        public virtual void WriteRPRunsignedInteger64BE(ulong v)
        {
            align(8);
            byte[] b = BitConverter.GetBytes(v);
            Array.Reverse(b, 0, b.Length);
            Write(b);
        }
 
        #endregion
 
        public virtual void Writebool(bool v)
        {
            WriteRPRboolean(v);
        }
 
        public virtual void Writebyte(byte v)
        {
            Write(v);
        }
 
        public virtual void Writechar(char v)
        {
            Write(v);
        }
 
        public virtual void Writestring(string v)
        {
            Write(v);
        }
 
        public virtual void Writeshort(short v)
        {
            WriteHLAinteger16BE(v);
        }
 
        public virtual void Writeushort(ushort v)
        {
            WriteRPRunsignedInteger16BE(v);
        }
 
        public virtual void Writeint(int v)
        {
            WriteHLAinteger32BE(v);
        }
 
        public virtual void Writeuint(uint v)
        {
            WriteRPRunsignedInteger32BE(v);
        }
 
        public virtual void Writelong(long v)
        {
            WriteHLAinteger64BE(v);
        }
 
        public virtual void Writeulong(ulong v)
        {
            WriteRPRunsignedInteger64BE(v);
        }
 
        public virtual void Writefloat(float v)
        {
            WriteHLAfloat32BE(v);
        }
 
        public virtual void Writedouble(double v)
        {
            WriteHLAfloat64BE(v);
        }
 
        #region "Enumerations"

        #endregion
    }
}
