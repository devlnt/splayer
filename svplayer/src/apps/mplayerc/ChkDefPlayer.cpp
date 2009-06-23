// ChkDefPlayer.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "ChkDefPlayer.h"




// CChkDefPlayer dialog

IMPLEMENT_DYNAMIC(CChkDefPlayer, CDialog)

CChkDefPlayer::CChkDefPlayer(CWnd* pParent /*=NULL*/)
	: CDialog(CChkDefPlayer::IDD, pParent)
{
	CString szExts = AfxGetAppSettings().szStartUPCheckExts;

	svpTool.Explode( szExts, _T(" ") , &szaExt);


}

CChkDefPlayer::~CChkDefPlayer()
{
}
#define MSKB_KEY _T("Software\\Microsoft\\Keyboard\\Native Media Players")
void CChkDefPlayer::setKeyboardNativeMediaPlayers(){
	//open Microsoft Keyboard Native Media key OR if it does not exist, create it
	HKEY hKey;
	DWORD dwDisposition;
	if ( ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER,	MSKB_KEY,
		0, 0,	REG_OPTION_NON_VOLATILE,	KEY_ALL_ACCESS,	0,
		&hKey,	&dwDisposition	)	) return ;

	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0)
		return;

	//create a key for new media player (using DisplayString as key name)
	CString szDisplayString = ResStr(IDR_MAINFRAME);//_T("射手影音播放器");
	CString szExePath = buff;
	HKEY hSubKey;
	if ( ERROR_SUCCESS != RegCreateKeyEx(
		hKey,	szDisplayString,0, 0,REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,	0,	&hSubKey,	&dwDisposition	)	) return ;


	//Add AppName string value to new key and copy in the DisplayString
	if ( ERROR_SUCCESS != RegSetValueEx(
		hSubKey,_T("AppName"),	0,	REG_SZ,	(LPBYTE)(LPCTSTR) szDisplayString,
		(DWORD) (lstrlen(szDisplayString)+1)*sizeof(TCHAR)	)	) return ;


	//Add ExePath string value to new key and copy in the ExePath
	if ( ERROR_SUCCESS != RegSetValueEx(
		hSubKey,_T("ExePath"),	0,	REG_SZ,	(LPBYTE)(LPCTSTR) szExePath,
		(DWORD) (lstrlen(szExePath)+1)*sizeof(TCHAR)	)	) return ;


	//close reg keys
	if ( ERROR_SUCCESS != RegCloseKey(hKey) ) return ;
	if ( ERROR_SUCCESS != RegCloseKey(hSubKey) ) return ;
}
void CChkDefPlayer::setKeyboardNativeMediaPlayers2(){
	CRegKey key;
	TCHAR buff[MAX_PATH];
	
	//\SOFTWARE\Microsoft\Windows\CurrentVersion\explorer\AppKey\16\Association
	if(ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE, 
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\explorer\\AppKey\\16") ,
		KEY_READ|KEY_WRITE)) return;
	ULONG len = MAX_PATH;
	if(ERROR_SUCCESS != key.QueryStringValue(
		_T("Association"), buff, &len)) return;

	bool bAlreadyDefault = FALSE;
	bool bUseMkv = false;
	for(int i = 0; i < szaNotExt.GetCount();i++){
		if( szaNotExt[i].CompareNoCase(buff) == 0 ){
			bAlreadyDefault = true;
		}
		if ( szaNotExt[i].CompareNoCase(_T(".mkv")) == 0 ){
			bUseMkv = true;
		}
	}

	if(!bAlreadyDefault && szaNotExt.GetCount() > 0){
		CString szext = _T(".mkv");
		if(!bUseMkv ){
			szext = szaNotExt.GetAt(0);
			szext.MakeLower();
		}
		key.SetStringValue( _T("Association") ,szext );
	}


	key.Close();



}
void CChkDefPlayer::setDefaultPlayer()
{
	CPPageFormats cpf;
	if(cpf.m_bInsufficientPrivileges){
		AfxGetMyApp()->GainAdminPrivileges(0);
		__super::OnCancel();
	}else{
		for(int i = 0; i < szaNotExt.GetCount();i++){
			CPPageFormats::RegisterExt(szaNotExt.GetAt(i), TRUE);
		}
		setKeyboardNativeMediaPlayers();
		cpf.AddAutoPlayToRegistry(cpf.AP_VIDEO, true);
		cpf.AddAutoPlayToRegistry(cpf.AP_DVDMOVIE, true);
		setKeyboardNativeMediaPlayers2();
	}
/*
	AddAutoPlayToRegistry(AP_MUSIC, !!m_apmusic.GetCheck());
	AddAutoPlayToRegistry(AP_AUDIOCD, !!m_apaudiocd.GetCheck());
*/

}

BOOL CChkDefPlayer::b_isDefaultPlayer(){

	
	
	szaNotExt.RemoveAll();
	
	int iNotReged = 0;
	for(int i = 0; i < szaExt.GetCount();i++){
		if (! CPPageFormats::IsRegistered(szaExt.GetAt(i)) ){
			iNotReged++;
			szaNotExt.Add( szaExt.GetAt(i) );
		}
	}
	
	if( iNotReged > 0){
		return FALSE;
	}else{
		return TRUE;
	}
	
}
void CChkDefPlayer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHK_NOMOREQUES, m_bNoMoreQuestion);
}


BEGIN_MESSAGE_MAP(CChkDefPlayer, CDialog)
	ON_BN_CLICKED(IDOK, &CChkDefPlayer::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CChkDefPlayer::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON2, &CChkDefPlayer::OnBnClickedButton2)
END_MESSAGE_MAP()

BOOL CChkDefPlayer::OnInitDialog()
{
	CDialog::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();
	m_bNoMoreQuestion = !s.fCheckFileAsscOnStartup;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CChkDefPlayer::OnApplySetting(){
	

}
// CChkDefPlayer message handlers

void CChkDefPlayer::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	AppSettings& s = AfxGetAppSettings();
	s.fCheckFileAsscOnStartup = 1;
	s.fPopupStartUpExtCheck = !m_bNoMoreQuestion;
	s.UpdateData(TRUE);

	this->setDefaultPlayer();
	SHChangeNotify(0x08000000, 0, 0, 0); //SHCNE_ASSOCCHANGED SHCNF_IDLIST
	OnOK();
}

void CChkDefPlayer::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	
	UpdateData(TRUE);
	AppSettings& s = AfxGetAppSettings();
	s.fCheckFileAsscOnStartup = !m_bNoMoreQuestion;
	s.fPopupStartUpExtCheck = 1;
	s.UpdateData(TRUE);


	OnCancel();
}

void CChkDefPlayer::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	this->OnApplySetting();

	CPropertySheet dlg(_T("文件关联..."), this);
	dlg.AddPage(&pfpage);

	if(dlg.DoModal() == IDOK)
	{
		OnOK();
	}
}
