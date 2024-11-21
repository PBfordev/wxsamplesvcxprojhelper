///////////////////////////////////////////////////////////////////////////////
// Project:     wxSamples VCXProj Helper
// Home:        https://github.com/PBfordev/wxsamplesvcxprojhelper
// File Name:   app.cpp
// Purpose:     Main frame and application
// Author:      PB
// Created:     2024-11-20
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/utils.h>

#include "data.h"

class MyFrame : public wxFrame
{
public:
    MyFrame(wxWindow* parent = nullptr);
private:
    wxString m_samplesRootFolder;
    ProjectDatas m_data;
    wxTextCtrl* m_logCtrl{nullptr};

    void OnLoadProjectsData(wxCommandEvent&);
    void OnWriteCmdFiles(wxCommandEvent&);
};


MyFrame::MyFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "Helper for MSVS vcxproj samples")
{
    wxConfigBase::Get()->Read("Samples Root Folder", &m_samplesRootFolder);

    SetMinClientSize(FromDIP(wxSize(800, 600)));
#ifdef __WXMSW__
    SetIcons(wxIconBundle("appIcon", nullptr));
#endif
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* buttonLoad = new wxButton(mainPanel, wxID_ANY, "Load projects data...");
    buttonLoad->Bind(wxEVT_BUTTON, &MyFrame::OnLoadProjectsData, this);
    buttonSizer->Add(buttonLoad, wxSizerFlags(1).Expand().Border());

    wxButton* buttonWrite = new wxButton(mainPanel, wxID_ANY, "Write .cmd files");
    buttonWrite->Bind(wxEVT_BUTTON, &MyFrame::OnWriteCmdFiles, this);
    buttonWrite->Bind(wxEVT_UPDATE_UI, [this](wxUpdateUIEvent& e)
        { e.Enable(!m_data.empty()); });
    buttonSizer->Add(buttonWrite, wxSizerFlags(1).Expand().Border());

    wxButton* buttonClearLog = new wxButton(mainPanel, wxID_ANY, "Clear Log");
    buttonClearLog->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        { m_logCtrl->Clear(); });
    buttonSizer->Add(buttonClearLog, wxSizerFlags().Expand().Border());

    mainPanelSizer->Add(buttonSizer, wxSizerFlags().Expand());

    m_logCtrl = new wxTextCtrl(mainPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));
    wxLog::DisableTimestamp();
    mainPanelSizer->Add(m_logCtrl, wxSizerFlags().Proportion(1).Expand().Border());

    mainPanel->SetSizer(mainPanelSizer);
}

void MyFrame::OnLoadProjectsData(wxCommandEvent&)
{
    m_data.clear();

    wxString dirName;

    if ( m_samplesRootFolder.empty() )
    {
        wxString str;
        wxFileName fn;

        if ( wxGetEnv("WXWIN", &str) )
        {
            fn.AssignDir(str);
            fn.AppendDir("samples");
            if ( fn.DirExists() )
                dirName = fn.GetFullPath();
        }
    }
    else
        dirName = m_samplesRootFolder;

    dirName = wxDirSelector("Samples Root Folder", dirName,
                            wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST, wxDefaultPosition, this);
    if ( dirName.empty() )
        return;

    m_samplesRootFolder = dirName;
    wxConfigBase::Get()->Write("Samples Root Folder", m_samplesRootFolder);

    static constexpr auto projectFileMask{ "*.bkl" };

    wxLogMessage("\n** Loading sample projects from folder '%s'...", m_samplesRootFolder);

    wxArrayString projectFiles;
    ProjectErrors errors;
    size_t warningCount{0};

    {
        wxBusyCursor busy;

        wxDir::GetAllFiles(m_samplesRootFolder, &projectFiles, projectFileMask, wxDIR_DEFAULT);
        if ( projectFiles.empty() )
        {
            wxLogMessage("No project files found.");
            return;
        }

        projectFiles.Sort();
        wxLogMessage("Processing %zu project files...", projectFiles.size());

        LoadProjectsData(projectFiles, m_data, errors, warningCount);
    }
    if ( !errors.empty() )
        wxBell();
    wxLogMessage("\n** Loaded %zu projects with data files, %zu errors, %zu warnings.\n",
        m_data.size(), errors.size(), warningCount);
}

void MyFrame::OnWriteCmdFiles(wxCommandEvent&)
{
    static constexpr auto cmdFileName{ "copyfiles.cmd" };

    wxLogMessage("\n** Writing command files for %zu projects...\n", m_data.size());
    ProjectErrors errors;

    {
        wxBusyCursor busy;

        size_t cmdFilesWritten{0};
        size_t warningCount{0};

        WriteCmdFiles(m_data, cmdFilesWritten, errors, warningCount);
        if ( !errors.empty() )
            wxBell();
        wxLogMessage("\n** Written %zu command files, %zu errors, %zu warnings.\n",
            cmdFilesWritten, errors.size(), warningCount);
    }

}

class MyApp : public wxApp
{
    bool OnInit() override
    {
        SetAppName("wxsamplesvcxprojhelper");
        SetVendorName("PB");

        (new MyFrame())->Show();
        return true;
    }
}; wxIMPLEMENT_APP(MyApp);