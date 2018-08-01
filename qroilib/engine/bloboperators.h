#ifndef BLOB_OPERATORS_H_INCLUDED
#define BLOB_OPERATORS_H_INCLUDED


#include <roilib_export.h>
#include "blob.h"

/**************************************************************************
		Definici?de les classes per a fer operacions sobre els blobs

		Helper classes to perform operations on blobs
**************************************************************************/

//! Factor de conversi?de graus a radians
#define DEGREE2RAD		(CV_PI / 180.0)


//! Classe d'on derivarem totes les operacions sobre els blobs
//! Interface to derive all blob operations
class ROIDSHARED_EXPORT COperadorBlob
{
public:
	virtual ~COperadorBlob(){};

	//! Aply operator to blob
	virtual double operator()(CBlob &blob) = 0;
	//! Get operator name
	virtual const char *GetNom() = 0;

	operator COperadorBlob*()
	{
		return (COperadorBlob*)this;
	}
};

typedef COperadorBlob funcio_calculBlob;

#ifdef BLOB_OBJECT_FACTORY
	/**
		Funci?per comparar dos identificadors dins de la f?rica de COperadorBlobs
	*/
	struct functorComparacioIdOperador
	{
	  bool operator()(const char* s1, const char* s2) const
	  {
		return strcmp(s1, s2) < 0;
	  }
	};

	//! Definition of Object factory type for COperadorBlob objects
	typedef ObjectFactory<COperadorBlob, const char *, functorComparacioIdOperador > t_OperadorBlobFactory;

	//! Funci?global per a registrar tots els operadors definits a blob.h
	void RegistraTotsOperadors( t_OperadorBlobFactory &fabricaOperadorsBlob );

#endif


//! Classe per calcular l'etiqueta d'un blob
//! Class to get ID of a blob
class ROIDSHARED_EXPORT CBlobGetID : public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{ 
		return blob.GetID(); 
	}
	const char *GetNom()
	{
		return "CBlobGetID";
	}
};


//! Classe per calcular l'?ea d'un blob
//! Class to get the area of a blob
class ROIDSHARED_EXPORT CBlobGetArea : public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{ 
		return blob.Area(); 
	}
	const char *GetNom()
	{
		return "CBlobGetArea";
	}
};

//! Classe per calcular el perimetre d'un blob
//! Class to get the perimeter of a blob
class ROIDSHARED_EXPORT CBlobGetPerimeter: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{ 
		return blob.Perimeter(); 
	}
	const char *GetNom()
	{
		return "CBlobGetPerimeter";
	}
};

//! Classe que diu si un blob ? extern o no
//! Class to get the extern flag of a blob
class ROIDSHARED_EXPORT CBlobGetExterior: public COperadorBlob
{
public:
	CBlobGetExterior()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExterior(IplImage *mask, bool xBorder = true, bool yBorder = true)
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{ 
		return blob.Exterior(m_mask, m_xBorder, m_yBorder); 
	}
	const char *GetNom()
	{
		return "CBlobGetExterior";
	}
private:
	IplImage *m_mask;
	bool m_xBorder, m_yBorder;
};

//! Classe per calcular la mitjana de nivells de gris d'un blob
//! Class to get the mean grey level of a blob
class ROIDSHARED_EXPORT CBlobGetMean: public COperadorBlob
{
public:
	CBlobGetMean()
	{
		m_iplImage = NULL;
	}
	CBlobGetMean( IplImage *image )
	{
		m_iplImage = image;
	};

    double operator()(CBlob &blob)
	{ 
		return blob.Mean(m_iplImage); 
	}
	const char *GetNom()
	{
		return "CBlobGetMean";
	}
private:

	IplImage *m_iplImage;
};

//! Classe per calcular la desviaci?est?dard dels nivells de gris d'un blob
//! Class to get the standard deviation of the grey level values of a blob
class ROIDSHARED_EXPORT CBlobGetStdDev: public COperadorBlob
{
public:
	CBlobGetStdDev()
	{
		m_iplImage = NULL;
	}
	CBlobGetStdDev( IplImage *image )
	{
		m_iplImage = image;
	};
    double operator()(CBlob &blob)
	{ 
		return blob.StdDev(m_iplImage); 
	}
	const char *GetNom()
	{
		return "CBlobGetStdDev";
	}
private:

	IplImage *m_iplImage;

};

//! Classe per calcular la compacitat d'un blob
//! Class to calculate the compactness of a blob
class ROIDSHARED_EXPORT CBlobGetCompactness: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetCompactness";
	}
};

//! Classe per calcular la longitud d'un blob
//! Class to calculate the length of a blob
class ROIDSHARED_EXPORT CBlobGetLength: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetLength";
	}
};

//! Classe per calcular l'amplada d'un blob
//! Class to calculate the breadth of a blob
class ROIDSHARED_EXPORT CBlobGetBreadth: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetBreadth";
	}
};

//! Classe per calcular la difer?cia en X del blob
class ROIDSHARED_EXPORT CBlobGetDiffX: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.GetBoundingBox().width;
	}
	const char *GetNom()
	{
		return "CBlobGetDiffX";
	}
};

//! Classe per calcular la difer?cia en X del blob
class ROIDSHARED_EXPORT CBlobGetDiffY: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.GetBoundingBox().height;
	}
	const char *GetNom()
	{
		return "CBlobGetDiffY";
	}
};

//! Classe per calcular el moment PQ del blob
//! Class to calculate the P,Q moment of a blob
class ROIDSHARED_EXPORT CBlobGetMoment: public COperadorBlob
{
public:
	//! Constructor est?dard
	//! Standard constructor (gets the 00 moment)
	CBlobGetMoment()
	{
		m_p = m_q = 0;
	}
	//! Constructor: indiquem el moment p,q a calcular
	//! Constructor: gets the PQ moment
	CBlobGetMoment( int p, int q )
	{
		m_p = p;
		m_q = q;
	};
	double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMoment";
	}

private:
	//! moment que volem calcular
	int m_p, m_q;
};

//! Classe per calcular el perimetre del poligon convex d'un blob
//! Class to calculate the convex hull perimeter of a blob
class ROIDSHARED_EXPORT CBlobGetHullPerimeter: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetHullPerimeter";
	}
};

//! Classe per calcular l'?ea del poligon convex d'un blob
//! Class to calculate the convex hull area of a blob
class ROIDSHARED_EXPORT CBlobGetHullArea: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetHullArea";
	}
};

//! Classe per calcular la x minima en la y minima
//! Class to calculate the minimum x on the minimum y
class ROIDSHARED_EXPORT CBlobGetMinXatMinY: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMinXatMinY";
	}
};

//! Classe per calcular la y minima en la x maxima
//! Class to calculate the minimum y on the maximum x
class ROIDSHARED_EXPORT CBlobGetMinYatMaxX: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMinYatMaxX";
	}
};

//! Classe per calcular la x maxima en la y maxima
//! Class to calculate the maximum x on the maximum y
class ROIDSHARED_EXPORT CBlobGetMaxXatMaxY: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMaxXatMaxY";
	}
};

//! Classe per calcular la y maxima en la x minima
//! Class to calculate the maximum y on the minimum y
class ROIDSHARED_EXPORT CBlobGetMaxYatMinX: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMaxYatMinX";
	}
};

//! Classe per a calcular la x m?ima
//! Class to get the minimum x
class ROIDSHARED_EXPORT CBlobGetMinX: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinX();
	}
	const char *GetNom()
	{
		return "CBlobGetMinX";
	}
};

//! Classe per a calcular la x m?ima
//! Class to get the maximum x
class ROIDSHARED_EXPORT CBlobGetMaxX: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MaxX();
	}
	const char *GetNom()
	{
		return "CBlobGetMaxX";
	}
};

//! Classe per a calcular la y m?ima
//! Class to get the minimum y
class ROIDSHARED_EXPORT CBlobGetMinY: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinY();
	}
	const char *GetNom()
	{
		return "CBlobGetMinY";
	}
};

//! Classe per a calcular la y m?ima
//! Class to get the maximum y
class ROIDSHARED_EXPORT CBlobGetMaxY: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MaxY();
	}
	const char *GetNom()
	{
		return "CBlobGetMaxY";
	}
};


//! Classe per calcular l'elongacio d'un blob
//! Class to calculate the elongation of the blob
class ROIDSHARED_EXPORT CBlobGetElongation: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetElongation";
	}
};

//! Classe per calcular la rugositat d'un blob
//! Class to calculate the roughness of the blob
class ROIDSHARED_EXPORT CBlobGetRoughness: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetRoughness";
	}
};

//! Classe per calcular la dist?cia entre el centre del blob i un punt donat
//! Class to calculate the euclidean distance between the center of a blob and a given point
class ROIDSHARED_EXPORT CBlobGetDistanceFromPoint: public COperadorBlob
{
public:
	//! Standard constructor (distance to point 0,0)
	CBlobGetDistanceFromPoint()
	{
		m_x = m_y = 0.0;
	}
	//! Constructor (distance to point x,y)
	CBlobGetDistanceFromPoint( const double x, const double y )
	{
		m_x = x;
		m_y = y;
	}

    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetDistanceFromPoint";
	}

private:
	// coordenades del punt on volem calcular la dist?cia
	double m_x, m_y;
};

//! Classe per calcular el nombre de pixels externs d'un blob
//! Class to get the number of extern pixels of a blob
class ROIDSHARED_EXPORT CBlobGetExternPerimeter: public COperadorBlob
{
public:
	CBlobGetExternPerimeter()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExternPerimeter( IplImage *mask, bool xBorder = true, bool yBorder = true )
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{
		return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder);
	}
	const char *GetNom()
	{
		return "CBlobGetExternPerimeter";
	}
private:
	IplImage *m_mask;
	bool m_xBorder, m_yBorder;
};

//! Classe per calcular el ratio entre el perimetre i nombre pixels externs
//! valors propers a 0 indiquen que la majoria del blob ? intern
//! valors propers a 1 indiquen que la majoria del blob ? extern
//! Class to calculate the ratio between the perimeter and the number of extern pixels
class ROIDSHARED_EXPORT CBlobGetExternPerimeterRatio: public COperadorBlob
{
public:
	CBlobGetExternPerimeterRatio()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExternPerimeterRatio( IplImage *mask, bool xBorder = true, bool yBorder = true )
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{
		if( blob.Perimeter() != 0 )
			return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder) / blob.Perimeter();
		else
			return blob.ExternPerimeter(m_mask,  m_xBorder, m_yBorder);
	}
	const char *GetNom()
	{
		return "CBlobGetExternPerimeterRatio";
	}
private:
	IplImage *m_mask;
	bool  m_xBorder, m_yBorder;
};

//! Classe per calcular el ratio entre el perimetre convex i nombre pixels externs
//! valors propers a 0 indiquen que la majoria del blob ? intern
//! valors propers a 1 indiquen que la majoria del blob ? extern
//! Class to calculate the ratio between the perimeter and the number of extern pixels
class ROIDSHARED_EXPORT CBlobGetExternHullPerimeterRatio: public COperadorBlob
{
public:
	CBlobGetExternHullPerimeterRatio()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExternHullPerimeterRatio( IplImage *mask, bool xBorder = true, bool yBorder = true )
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{
		CBlobGetHullPerimeter getHullPerimeter;
		double hullPerimeter;

		if( (hullPerimeter = getHullPerimeter( blob ) ) != 0 )
			return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder) / hullPerimeter;
		else
			return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder);
	}
	const char *GetNom()
	{
		return "CBlobGetExternHullPerimeterRatio";
	}
private:
	IplImage *m_mask;
	bool  m_xBorder, m_yBorder;

};

//! Classe per calcular el centre en el eix X d'un blob
//! Class to calculate the center in the X direction
class ROIDSHARED_EXPORT CBlobGetXCenter: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinX() + (( blob.MaxX() - blob.MinX() ) / 2.0);
	}
	const char *GetNom()
	{
		return "CBlobGetXCenter";
	}
};

//! Classe per calcular el centre en el eix Y d'un blob
//! Class to calculate the center in the Y direction
class ROIDSHARED_EXPORT CBlobGetYCenter: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinY() + (( blob.MaxY() - blob.MinY() ) / 2.0);
	}
	const char *GetNom()
	{
		return "CBlobGetYCenter";
	}
};

//! Classe per calcular la longitud de l'eix major d'un blob
//! Class to calculate the length of the major axis of the ellipse that fits the blob edges
class ROIDSHARED_EXPORT CBlobGetMajorAxisLength: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CvBox2D elipse = blob.GetEllipse();

		return elipse.size.width;
	}
	const char *GetNom()
	{
		return "CBlobGetMajorAxisLength";
	}
};

//! Classe per calcular el ratio entre l'area de la elipse i la de la taca
//! Class 
class ROIDSHARED_EXPORT CBlobGetAreaElipseRatio: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		if( blob.Area()==0.0 ) return 0.0;

		CvBox2D elipse = blob.GetEllipse();
		double ratioAreaElipseAreaTaca = ( (elipse.size.width/2.0)
										   *
										   (elipse.size.height/2.0)
							               *CV_PI
						                 )
									     /
									     blob.Area();

		return ratioAreaElipseAreaTaca;
	}
	const char *GetNom()
	{
		return "CBlobGetAreaElipseRatio";
	}
};

//! Classe per calcular la longitud de l'eix menor d'un blob
//! Class to calculate the length of the minor axis of the ellipse that fits the blob edges
class ROIDSHARED_EXPORT CBlobGetMinorAxisLength: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CvBox2D elipse = blob.GetEllipse();

		return elipse.size.height;
	}
	const char *GetNom()
	{
		return "CBlobGetMinorAxisLength";
	}
};

//! Classe per calcular l'orientaci?de l'ellipse del blob en radians
//! Class to calculate the orientation of the ellipse that fits the blob edges in radians
class ROIDSHARED_EXPORT CBlobGetOrientation: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CvBox2D elipse = blob.GetEllipse();
/*
		if( elipse.angle > 180.0 )
			return (( elipse.angle - 180.0 )* DEGREE2RAD);
		else
			return ( elipse.angle * DEGREE2RAD);
*/
		return elipse.angle;
	}
	const char *GetNom()
	{
		return "CBlobGetOrientation";
	}
};

//! Classe per calcular el cosinus de l'orientaci?de l'ellipse del blob
//! Class to calculate the cosinus of the orientation of the ellipse that fits the blob edges
class ROIDSHARED_EXPORT CBlobGetOrientationCos: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CBlobGetOrientation getOrientation;
		return fabs( cos( getOrientation(blob)*DEGREE2RAD ));
	}
	const char *GetNom()
	{
		return "CBlobGetOrientationCos";
	}
};


//! Classe per calcular el ratio entre l'eix major i menor de la el·lipse
//! Class to calculate the ratio between both axes of the ellipse
class ROIDSHARED_EXPORT CBlobGetAxisRatio: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		double major,minor;
		CBlobGetMajorAxisLength getMajor;
		CBlobGetMinorAxisLength getMinor;

		major = getMajor(blob);
		minor = getMinor(blob);

		if( major != 0 )
			return minor / major;
		else
			return 0;
	}
	const char *GetNom()
	{
		return "CBlobGetAxisRatio";
	}
};


//! Classe per calcular si un punt cau dins del blob
//! Class to calculate whether a point is inside a blob
class ROIDSHARED_EXPORT CBlobGetXYInside: public COperadorBlob
{
public:
	//! Constructor est?dard
	//! Standard constructor
	CBlobGetXYInside()
	{
		m_p.x = 0;
		m_p.y = 0;
	}
	//! Constructor: indiquem el punt
	//! Constructor: sets the point
	CBlobGetXYInside( CvPoint2D32f p )
	{
		m_p = p;
	};
	double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetXYInside";
	}

private:
	//! punt que considerem
	//! point to be considered
	CvPoint2D32f m_p;
};

#endif	//!BLOB_OPERATORS_H_INCLUDED
