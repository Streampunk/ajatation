
// AjatationTestControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AjatationTestControl.h"
#include "AjatationTestControlDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAjatationTestControlDlg dialog



CAjatationTestControlDlg::CAjatationTestControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAjatationTestControlDlg::IDD, pParent),
    control(CAjatationTestControlDlg::UpdateCallback, this)
    , captureDevice(0)
    , captureChannel(0)
    , playbackDevice(0)
    , playbackChannel(0)
    , capFramesReceived(0)
    , capBytesPerFrame(0)
    , capFramesAvailable(0)
    , pbFramesSent(0)
    , pbBytesPerFrame(0)
    , pcFramesAvailable(0)
    , capStatus(_T(""))
    , pbStatus(_T(""))
    , routeFrames(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAjatationTestControlDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_CAP_DEVICE, captureDevice);
    DDV_MinMaxUInt(pDX, captureDevice, 0, 1000);
    DDX_Text(pDX, IDC_EDIT_CAP_CHANNEL, captureChannel);
    DDV_MinMaxUInt(pDX, captureChannel, 0, 1000);
    DDX_Text(pDX, IDC_EDIT_PB_DEVICE, playbackDevice);
    DDV_MinMaxUInt(pDX, playbackDevice, 0, 1000);
    DDX_Text(pDX, IDC_EDIT_PB_CHANNEL, playbackChannel);
    DDV_MinMaxUInt(pDX, playbackChannel, 0, 1000);
    DDX_Text(pDX, IDC_CAPTURE_FRAMES, capFramesReceived);
    DDX_Text(pDX, IDC_CAPTURE_FRAME_SIZE, capBytesPerFrame);
    DDX_Text(pDX, IDC_CAP_FRAMES_AVAILABLE, capFramesAvailable);
    DDX_Text(pDX, IDC_PLAYBACK_FRAMES, pbFramesSent);
    DDX_Text(pDX, IDC_PLAYBACK_FRAME_SIZE, pbBytesPerFrame);
    DDX_Text(pDX, IDC_PB_FRAMES_AVAILABLE, pcFramesAvailable);
    DDX_Text(pDX, IDC_CAPTURE_STATUS, capStatus);
    DDX_Text(pDX, IDC_PLAYBACK_STATUS, pbStatus);
    DDX_Check(pDX, IDC_CHECK1, routeFrames);
}

BEGIN_MESSAGE_MAP(CAjatationTestControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
    ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
    ON_STN_CLICKED(IDC_CAPTURE_FRAMES, &CAjatationTestControlDlg::OnStnClickedCaptureFrames)
    ON_BN_CLICKED(IDC_BUTTON_START_CAPTURE, &CAjatationTestControlDlg::OnBnClickedButtonStartCapture)
    ON_BN_CLICKED(IDC_BUTTON_START_PLAYBACK, &CAjatationTestControlDlg::OnBnClickedButtonStartPlayback)
    ON_MESSAGE(AJA_STATUS_UPDATE, OnStatusUpdate)
    ON_EN_CHANGE(IDC_EDIT_CAP_CHANNEL, &CAjatationTestControlDlg::OnEnChangeEditCapChannel)
    ON_BN_CLICKED(IDC_CHECK1, &CAjatationTestControlDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()



LRESULT CAjatationTestControlDlg::OnStatusUpdate(WPARAM w, LPARAM l)
{
    AjaController::Status status;

    control.GetStatus(status);

    capFramesReceived = status.captureFrames;
    capBytesPerFrame = status.captureFrameSize;
    capFramesAvailable = status.captureFramesAvailable;
    pbFramesSent = status.playbackFrames;
    pbBytesPerFrame = status.playbackFrameSize;
    pcFramesAvailable = status.playbackFramesAvailable;
    capStatus = status.capturing ? "Capturing" : "Stopped";
    pbStatus = status.playing ? "Playing" : "Stopped";

    UpdateData(0);

    return 1;
}

// CAjatationTestControlDlg message handlers

BOOL CAjatationTestControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAjatationTestControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAjatationTestControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAjatationTestControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAjatationTestControlDlg::OnStnClickedCaptureFrames()
{
    // TODO: Add your control notification handler code here
}


void CAjatationTestControlDlg::OnBnClickedButtonStartCapture()
{
    UpdateData();

    if(!control.IsCapturing())
    {
        control.StartCapture();
    }
    else
    {
        control.StopCapture();
    }
}


void CAjatationTestControlDlg::OnBnClickedButtonStartPlayback()
{
    UpdateData();


    if(!control.IsPlaying())
    {
        control.StartPlayback();
    }
    else
    {
        control.StopPlayback();
    }
}


void CAjatationTestControlDlg::UpdateCallback(void* context)
{
    reinterpret_cast<CAjatationTestControlDlg*>(context)->UpdateCallback();
}


void CAjatationTestControlDlg::UpdateCallback()
{
    this->PostMessageW(AJA_STATUS_UPDATE);
}


void CAjatationTestControlDlg::OnEnChangeEditCapChannel()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}


void CAjatationTestControlDlg::OnBnClickedCheck1()
{
    UpdateData();

    control.CaptureToPlaybackRouting((bool) routeFrames);
}
