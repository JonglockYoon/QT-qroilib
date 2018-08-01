
#include "blob.h"
#include "blobproperties.h"

/**
- FUNCI? GetPerimeter
- FUNCIONALITAT: Get perimeter from chain code. Diagonals sum sqrt(2) and horizontal and vertical codes 1
- PARÀMETRES:
	- 
- RESULTAT:
	- 
- RESTRICCIONS:
	- 
- AUTOR: rborras
- DATA DE CREACI? 2008/04/30
- MODIFICACI? Data. Autor. Descripci?
- NOTA: Algorithm derived from "Methods to estimate area and perimeters of blob-like objects: A comparison", L.Yang
*/
#define SQRT2 1.414213562

/**
- FUNCI? GetPerimeter
- FUNCIONALITAT: Get blob area, ie. external contour area minus internal contours area
- PARÀMETRES:
	- 
- RESULTAT:
	- 
- RESTRICCIONS:
	- 
- AUTOR: rborras
- DATA DE CREACI? 2008/04/30
- MODIFICACI? Data. Autor. Descripci?
*/
double CBlobProperties::GetArea()
{
	double area;
	t_contourList::iterator itContour; 

	area = m_externalContour.GetArea();

	itContour = m_internalContours.begin();
	
	while (itContour != m_internalContours.end() )
	{
		area += (*itContour).GetArea();
		itContour++;
	}
	return area;
}

/**
- FUNCI? GetPerimeter
- FUNCIONALITAT: Get blob perimeter, ie. sum of the lenght of all the contours
- PARÀMETRES:
	- 
- RESULTAT:
	- 
- RESTRICCIONS:
	- 
- AUTOR: rborras
- DATA DE CREACI? 2008/04/30
- MODIFICACI? Data. Autor. Descripci?
*/
double CBlobProperties::GetPerimeter()
{
	double perimeter;
	t_contourList::iterator itContour; 

	perimeter = m_externalContour.GetPerimeter();

	itContour = m_internalContours.begin();
	
	while (itContour != m_internalContours.end() )
	{
		perimeter += (*itContour).GetPerimeter();
		itContour++;
	}
	return perimeter;
}


