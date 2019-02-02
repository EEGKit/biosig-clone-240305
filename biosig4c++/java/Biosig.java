/* 
    Copyright (C) 2018,2019 Alois Schloegl <alois.schloegl@gmail.com>
    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/


    BioSig is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    https://www3.ntu.edu.sg/home/ehchua/programming/java/JavaNativeInterface.html
*/

//package Biosig;
public class Biosig {
    static { 
    	System.loadLibrary("Biosig"); 
    }

    private native void version();
    private native String hdr2ascii(String filename);
    private native void hdr2json(String filename);
    
    private native void sopen(String filename);
    private native void sread(String filename);
    private native void sclose(String filename);

    public static void main(String[] args) {
         System.out.println(new Biosig().hdr2ascii(args[0]));
    }
}


