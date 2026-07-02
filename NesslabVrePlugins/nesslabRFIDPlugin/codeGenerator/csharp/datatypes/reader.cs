#region --- License ---
/*********************************************************************************
** Copyright (c) 1992-2025 MAK Technologies, Inc
** All rights reserved.
*********************************************************************************/
#endregion
/* reader.tlt */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!This is an auto-generated file.  Any changes will be destroyed!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
using System;
using System.IO;
namespace makVrl
{
	public class HLAReader : BinaryReader
	{
        private byte[] a; // for little endian reversals
 
        public HLAReader(Stream instream)
            : base(instream, System.Text.Encoding.Unicode)
        {
 
        }
 
        // Skip implicit padding bytes
        public virtual void align(int boundary)
        {
            int mis = (int) (BaseStream.Position % (long) boundary);
            if (mis > 0)
            {
                ReadBytes(boundary - mis);
            }
        }
 
        public virtual void ReadPadding(int n)
        {
            ReadBytes(n);
        }
 
        #region "HLA datatypes"
 
        public virtual byte ReadHLAoctet()
        {
            return ReadByte();
        }
 
        public virtual char ReadHLAoctetPairLE()
        {
            align(2);
            return ReadChar();
        }
 
        public virtual char ReadHLAoctetPairBE()
        {
            int len = 2;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToChar(a, 0);
        }
 
        public virtual short ReadHLAinteger16LE()
        {
            align(2);
            return ReadInt16();
        }
 
        public virtual short ReadHLAinteger16BE()
        {
            int len = 2;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToInt16(a, 0);
        }
 
        public virtual int ReadHLAinteger32LE()
        {
            int len = 4;
            align(len);
            return ReadInt32();
        }
 
        public virtual int ReadHLAinteger32BE()
        {
            int len = 4;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToInt32(a, 0);
        }
 
        public virtual long ReadHLAinteger64LE()
        {
            int len = 8;
            align(len);
            return ReadInt64();
        }
 
        public virtual long ReadHLAinteger64BE()
        {
            int len = 8;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToInt64(a, 0);
        }
 
        public virtual float ReadHLAfloat32LE()
        {
            int len = 4;
            align(len);
            return ReadSingle(); 
        }
 
        public virtual float ReadHLAfloat32BE()
        {
            int len = 4;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToSingle(a, 0);
        }
 
        public virtual double ReadHLAfloat64LE()
        {
            int len = 8;
            align(len);
            return ReadDouble();
        }
 
        public virtual double ReadHLAfloat64BE()
        {
            int len = 8;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToDouble(a, 0);
        }
 
        #endregion
 
        #region "RPR datatypes"
 
        public virtual byte ReadRPRunsignedInteger8BE()
        {
            return ReadByte();
        }
 
        public virtual ushort ReadRPRunsignedInteger16BE()
        {
            int len = 2;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToUInt16(a, 0);
        }
 
        public virtual uint ReadRPRunsignedInteger32BE()
        {
            int len = 4;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToUInt32(a, 0);
        }
 
        public virtual ulong ReadRPRunsignedInteger64BE()
        {
            int len = 8;
            align(len);
            a = ReadBytes(len);
            Array.Reverse(a, 0, len);
            return BitConverter.ToUInt64(a, 0);
        }
 
        #endregion
 
        public virtual bool Readbool()
        {
            return ReadByte() == 0 ? false : true;
        }
 
        public virtual byte Readbyte()
        {
            return ReadByte();
        }
 
        public virtual char Readchar()
        {
            return ReadChar();
        }
 
        public virtual string Readstring()
        {
            return "";
        }
 
        public virtual short Readshort()
        {
            return ReadHLAinteger16BE();
        }
 
        public virtual ushort Readushort()
        {
            return ReadRPRunsignedInteger16BE();
        }
 
        public virtual int Readint()
        {
            return ReadHLAinteger32BE();
        }
 
        public virtual uint Readuint()
        {
            return ReadRPRunsignedInteger32BE();
        }
 
        public virtual long Readlong()
        {
            return ReadHLAinteger64BE();
        }
 
        public virtual ulong Readulong()
        {
            return ReadRPRunsignedInteger64BE();
        }
 
        public virtual float Readfloat()
        {
            return ReadHLAfloat32BE();
        }
 
        public virtual double Readdouble()
        {
            return ReadHLAfloat64BE();
        }
 
        #region "Enumerations"

        #endregion
    }
}
