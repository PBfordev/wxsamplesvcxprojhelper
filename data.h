///////////////////////////////////////////////////////////////////////////////
// Project:     wxSamples VCXProj Helper
// Home:        https://github.com/PBfordev/wxsamplesvcxprojhelper
// File Name:   data.h
// Purpose:     Reading project files and writing command files
// Author:      PB
// Created:     2024-11-20
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <wx/arrstr.h>
#include <wx/string.h>

#include <map>

class ProjectData
{
public:
    ProjectData(const wxString& projectPath);
    ProjectData(const wxString& projectPath,
                const wxString& dstDir, const wxString& srcDir,
                const wxArrayString& dataFiles);
    void AssignDirs(const wxString& dstDir, const wxString& srcDir);
    void AssignDataFiles(const wxArrayString& dataFiles);

    // hack for projects that cannot be loaded
    // currently known only internat
    void ForceXCopyCommands(const wxArrayString& commands);
    bool HasForcedXCopyCommands() const { return !m_forcedXCopyCommands.empty(); }

    bool HasFiles() const { return !m_dataFiles.empty(); }

    wxString GetProjectPath() const { return m_projectPath; }
    wxString GetSrcDir() const { return m_srcDir; }
    wxArrayString GetFileNames() const { return m_dataFiles; };
    wxArrayString GetXCopyCommands() const;
private:
    wxArrayString m_forcedXCopyCommands;
    wxString m_projectPath;
    wxString m_dstDir, m_srcDir;
    wxArrayString m_dataFiles;
};

// the key is the full project file path
using ProjectDatas = std::vector<ProjectData>;

struct ProjectError
{
    ProjectError(const wxString& path, const wxArrayString& errors = wxArrayString())
        : m_path(path), m_errors(errors)
    {}
    wxString m_path;
    wxArrayString m_errors;
};

using ProjectErrors = std::vector<ProjectError>;

void LoadProjectsData(const wxArrayString& projectsPaths, ProjectDatas& datas,
                      ProjectErrors& errors, size_t& warningCount);

void WriteCmdFiles(const ProjectDatas& datas, size_t& cmdFilesWritten,
                   ProjectErrors& errors, size_t& warningCount);