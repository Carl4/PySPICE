/*
 * spicemodule_cust.om.c
 *
 *  Created on: Jun 24, 2012
 *      Author: Daddy
 */

#include "pyspice.h"
#include <string.h>
#include "spicemodule_custom.h"

PyObject *SpiceException;

/* void spkw10_c ( SpiceInt handle,SpiceInt body,SpiceInt center,
ConstSpiceChar * frame,SpiceDouble first,SpiceDouble last,
ConstSpiceChar * segid,ConstSpiceDouble consts [8],SpiceInt n,
ConstSpiceDouble elems [],ConstSpiceDouble epochs [] ); */
PyObject * spice_spkw10(PyObject *self, PyObject *args)
{
  /* variables for inputs */
  long handle;
  long body;
  long center;
  char * frame;
  double first;
  double last;
  char * segid;
  double consts[8];
  long n;
  PyObject * elems;
  PyObject * epochs;
  PyObject * elem_seq;
  PyObject * epoch_seq;
  PyObject * elem;
  int i, j	;
  double * c_elems;
  double * c_epochs;

  char failed = 0;

  PYSPICE_CHECK_RETURN_STATUS(PyArg_ParseTuple(args, "lllsdds(dddddddd)OO",
		  &handle, &body, &center, &frame, &first, &last, &segid,
		  &consts[0], &consts[1], &consts[2], &consts[3], &consts[4],
		  &consts[5], &consts[6], &consts[7], &elems, &epochs));

  // This will fail.  Allocation and translation for elems and epoch.
  //spkw10_c(handle, body, center, frame, first, last, segid, consts, n, &elems, &epochs);
  /* Need to do the following:
   * 1: Allocate memory for elems and epochs
   * 2: Populate elems and epochs from the python iterables that get passed in.
   */
  elem_seq = PySequence_Fast(elems, "expected a sequence for elems");
  epoch_seq = PySequence_Fast(epochs, "expected a sequence for epochs");

  n = PySequence_Size(elem_seq);
  if (n != PySequence_Size(epoch_seq)) {
	  // TODO: How to raise an exception??
	  printf("Naa na naa na naa\nNa na naa na naa\nYou're gonna _____! ");
	  return NULL;
  }
  // How to properly allocate memory??  Am I allowed to use malloc?
  // See: http://article.gmane.org/gmane.comp.python.general/424736
  c_elems = (double*) PyMem_Malloc(n * 10 * sizeof(double));
  c_epochs = (double*) PyMem_Malloc(n * sizeof(double));

  for (i = 0; i < n; i++) {
	  c_epochs[i] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(epoch_seq, i));
	  elem = PySequence_Fast_GET_ITEM(elem_seq, i);

	  for (j = 0 ; j < 10 ; j++) {
		  c_elems[i*10+j] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(elem, j));
	  }

	  /* DON'T DECREF item here */
	  //Py_DECREF(elem);
	  //Py_DECREF(epoch);
  }

  Py_DECREF(elem_seq);
  Py_DECREF(epoch_seq);

  spkw10_c(handle, body, center, frame, first, last, segid, consts, n,
		   c_elems, c_epochs);

  PyMem_Free(c_elems);
  PyMem_Free(c_epochs);

  PYSPICE_CHECK_FAILED;

  if(failed) {
    return NULL;
  }


  Py_INCREF(Py_None);
  return Py_None;
}
PyDoc_STRVAR(spkw10_doc, "-Abstract\n\n   Write an SPK type 10 segment to the DAF open and attached to\n   the input handle.\n\n-Brief_I/O\n\n   VARIABLE  I/O  DESCRIPTION\n   --------  ---  --------------------------------------------------\n   handle     I   The handle of a DAF file open for writing.\n   body       I   The NAIF ID code for the body of the segment.\n   center     I   The center of motion for body.\n   frame      I   The reference frame for this segment.\n   first      I   The first epoch for which the segment is valid.\n   last       I   The last  epoch for which the segment is valid.\n   segid      I   The string to use for segment identifier.\n   consts     I   The array of geophysical constants for the segment\n # REMOVED:  n          I   The number of element/epoch pairs to be stored\n   elems      I   The collection of \"two-line\" element sets.\n   epochs     I   The epochs associated with the element sets.\n\n-Detailed_Output\n\n   None.       The data input is stored in an SPK segment in the\n               DAF connected to the input handle.\n\n");

/* void getelm_c ( SpiceInt         frstyr,
                   SpiceInt         lineln,
                   const void     * lines,
                   SpiceDouble    * epoch,
                   SpiceDouble    * elems   ) */
PyObject * spice_getelm(PyObject *self, PyObject *args)
{
  /* variables for inputs */
  long firstyr;
  long lineln;
  long line1_len, line2_len;
  char * line1;
  char * line2;
  char * f_lines;
  size_t bufsize;

  double elems[10];
  double epoch;

  char failed = 0;

  PYSPICE_CHECK_RETURN_STATUS(PyArg_ParseTuple(args, "l(ss)",
		  &firstyr, &line1, &line2));

  // How to properly allocate memory??  Am I allowed to use malloc?
  // See: http://article.gmane.org/gmane.comp.python.general/424736
  line1_len = strlen(line1);
  line2_len = strlen(line2);
  
  bufsize = (size_t) 2 + line1_len + line2_len;
  f_lines = PyMem_Malloc(bufsize);  

  if (f_lines == NULL) {
    return PyErr_NoMemory();
  }

  // Offset to the start of the second line.
  lineln = line1_len+2;
  // Add 1 to line_len to be sure we copy the null too.
  strncpy(f_lines, line1, line1_len+2);
  strncpy(f_lines+lineln, line2, line2_len+2);
  getelm_c(firstyr, lineln, f_lines, &epoch, elems);
 
  PyMem_Free(f_lines);

  PYSPICE_CHECK_FAILED;
  
  if(failed) {
    return NULL;
  }

  PyObject *returnVal = Py_BuildValue("d(dddddddddd)", epoch,\
		  elems[0], elems[1], elems[2],\
		  elems[3], elems[4], elems[5],\
		  elems[6], elems[7], elems[8],\
		  elems[9] );
  return returnVal;


  Py_INCREF(Py_None);
  return Py_None;
}
PyDoc_STRVAR(getelm_doc, "-Brief_I/O\
 \
   VARIABLE  I/O  DESCRIPTION\
   --------  ---  --------------------------------------------------\
   frstyr     I   Year of earliest representable two-line elements.\
   lineln     I   Length of strings in lines array.\
   lines      I   A pair of ''lines'' containing two-line elements.\
   epoch      O   The epoch of the elements in seconds past J2000. \
   elems      O   The elements converted to SPICE units.\
\
-Detailed_Input\
\
   frstyr    is the first year possible for two line elements. Since\
             two line elements allow only two digits for the year, some\
             conventions must be followed concerning which century the\
             two digits refer to.  frstyr is the year of the earliest\
             representable elements. The two-digit year is mapped to\
             the year in the interval from frstyr to frstyr + 99 that\
             has the same last two digits as the two digit year in the\
             element set.  For example if frstyr is set to 1960  then\
             the two digit years are mapped as shown in the table\
             below:\
\
             Two-line         Maps to\
             element year\
\
                00            2000\
                01            2001\
                02            2002\
                 .              .\
                 .              .\
                 .              .\
                58            2058 \
                59            2059\
               -------------------- \
                60            1960\
                61            1961\
                62            1962\
                 .              .\
                 .              .\
                 .              .\
                99            1999 \
\
              Note that if Space Command should decide to represent\
              years in 21st century as 100 + the last two digits of the\
              year (for example: 2015 is represented as 115) instead of\
              simply dropping the first two digits of the year, this\
              routine will correctly map the year as long as you set\
              frstyr to some value between 1900 and 1999.\
\
   lines      is a pair of lines of text that comprise a Space command\
              ``two-line element'' set.  lines should be declared\
\
                 SpiceChar lines[2][lineln];\
\
              These text lines should be the same as they are presented\
              in the two-line element files available from Space\
              Command (formerly NORAD). Below is an example of a\
              two-line set for TOPEX.\
\
   TOPEX\
   1 22076U 92052A   97173.53461370 -.00000038  00000-0  10000-3 0   594\
   2 22076  66.0378 163.4372 0008359 278.7732  81.2337 12.80930736227550\
\
\
\
\
-Detailed_Output\
\
   epoch      is the epoch of the two line elements supplied via\
              the input array lines.  Epoch is returned in TDB\
              seconds past J2000.\
\
   elems      is an array containing the elements from the two line\
              set supplied via the array lines.  The elements are\
              in units suitable for use by the CSPICE routine\
              ev2lin_. \
\
              Also note that the elements XNDD6O and BSTAR\
              incorporate the exponential factor present in the\
              input two line elements in LINES.  (See particulars\
              below.\
\
                  ELEMS [ 0 ] = XNDT2O in radians/minute**2\
                  ELEMS [ 1 ] = XNDD6O in radians/minute**3\
                  ELEMS [ 2 ] = BSTAR\
                  ELEMS [ 3 ] = XINCL  in radians\
                  ELEMS [ 4 ] = XNODEO in radians\
                  ELEMS [ 5 ] = EO\
                  ELEMS [ 6 ] = OMEGAO in radians\
                  ELEMS [ 7 ] = XMO    in radians\
                  ELEMS [ 8 ] = XNO    in radians/minute\
                  ELEMS [ 9 ] = EPOCH of the elements in seconds\
                                past ephemeris epoch J2000.\
\
-Parameters\
\
   None.\
\
-Exceptions\
\
   No checking of the inputs is performed in this routine. However, this\
   routine does call other CSPICE routines. If one of these routines\
   detects an error it will diagnose it and signal an error.\
\
-Files\
\
   You must have loaded a SPICE leapseconds kernel into the\
   kernel pool prior to caling this routine.\
\
-Particulars\
\
   This routine parses a Space Command Two-line element set and returns\
   the orbital elements properly scaled and in units suitable for use\
   by other SPICE software.  Input elements look like the following\
\
---------------------------------------------------------------------\
1 22076U 92052A   97173.53461370 -.00000038  00000-0  10000-3 0   594\
2 22076  66.0378 163.4372 0008359 278.7732  81.2337 12.80930736227550\
---------------------------------------------------------------------\
^\
123456789012345678901234567890123456789012345678901234567890123456789\
         1         2         3         4         5         6\
\
   The ``raw'' elements in the first and second lines are marked below.\
   Note that in several instances exponents and decimal points are\
   implied.  Also note that input units are degrees, degrees/day**n and\
   revolutions/day.\
\
\
                    DAY OF YEAR             NDD60    BSTAR\
                    vvvvvvvvvvvv            vvvvvv   vvvvvv \
---------------------------------------------------------------------\
1 22076U 92052A   97173.53461370 -.00000038  00000-0  10000-3 0   594\
---------------------------------------------------------------------\
                  ^^             ^^^^^^^^^^       ^^       ^^\
                  YEAR             NDT20          IEXP     IBEXP\
\
\
\
   The ``raw'' elements in the second line are marked below\
                 NODE0            OMEGA             N0\
                 vvvvvvvv         vvvvvvvv          vvvvvvvvvvv\
---------------------------------------------------------------------\
2 22076  66.0378 163.4372 0008359 278.7732  81.2337 12.80930736227550\
---------------------------------------------------------------------\
        ^^^^^^^^          ^^^^^^^          ^^^^^^^^\
        Inclination       Eccentricity     M0\
\
   This routine extracts these values ``inserts'' the implied\
   decimal points and exponents and then converts the inputs\
   to units of radians, radians/minute, radians/minute**2, and\
   radians/minute**3");

/*
PyMethodDef custom_methods[] = {
    {"spkw10", spice_spkw10, METH_VARARGS, spkw10_doc},
    {"getelm", spice_getelm, METH_VARARGS, getelm_doc},
	{NULL, NULL},

};


void init_spice(PyObject *self)
{
  PyObject *m = NULL;

  m = Py_InitModule("_spice", custom_methods);

  // on't allow an exception to stop execution
  erract_c("SET", 0, "RETURN");
  errdev_c("SET", 0, "NULL");

  SpiceException =     PyErr_NewException("_spice.SpiceException", PyExc_Exception, NULL);
  Py_INCREF(SpiceException);

  PyModule_AddObject(m, "SpiceException", SpiceException);
}
*/
