// CS580HWDoc.h : interface of the CCS580HWDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CS580HWDOC_H__C28F4603_B299_4DB1_9E4D_835F8AA79918__INCLUDED_)
#define AFX_CS580HWDOC_H__C28F4603_B299_4DB1_9E4D_835F8AA79918__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CCS580HWDoc : public CDocument
{
protected: // create from serialization only
	CCS580HWDoc();
	DECLARE_DYNCREATE(CCS580HWDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCS580HWDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCS580HWDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCS580HWDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRenderGenmesh();
	afx_msg void OnRenderGenprojmesh();
	void LinePlaneIntersection(float hPoint1[4], float hPoint2[4], float hPoint[4]);
	void InterpolateHomoCoord(int i, int j, float PointWorld[4]);
	float m_LeftBottom[4], m_RightBottom[4], m_LeftTop[4], m_RightTop[4];
	int xNum,yNum,zNum;
	int xDim,yDim,zDim;
	float xRes, yRes, zRes;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CS580HWDOC_H__C28F4603_B299_4DB1_9E4D_835F8AA79918__INCLUDED_)
