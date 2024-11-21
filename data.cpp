///////////////////////////////////////////////////////////////////////////////
// Project:     wxSamples VCXProj Helper
// Home:        https://github.com/PBfordev/wxsamplesvcxprojhelper
// File Name:   data.cpp
// Purpose:     Reading project files and writing command files
// Author:      PB
// Created:     2024-11-20
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////


#include <wx/wx.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/xml/xml.h>

#include "data.h"

ProjectData::ProjectData(const wxString& projectPath)
   : m_projectPath(projectPath)
{}

ProjectData::ProjectData(const wxString& projectPath,
                         const wxString& dstDir, const wxString& srcDir,
                         const wxArrayString& dataFiles)
    : m_projectPath(projectPath),
      m_dstDir(dstDir), m_srcDir(srcDir), m_dataFiles(dataFiles)
{}

void ProjectData::AssignDirs(const wxString& dstDir, const wxString& srcDir)
{
    m_dstDir = dstDir;
    m_srcDir = srcDir;
}

void ProjectData::AssignDataFiles(const wxArrayString& dataFiles)
{
    m_dataFiles = dataFiles;
}

void ProjectData::ForceXCopyCommands(const wxArrayString& commands)
{
    m_forcedXCopyCommands = commands;
}

wxArrayString ProjectData::GetXCopyCommands() const
{
    wxArrayString commands;

    if ( !m_forcedXCopyCommands.empty() )
        commands = m_forcedXCopyCommands;

    wxString command;
    // dirs with a slash added
    wxString dstDir(m_dstDir), srcDir(m_srcDir);

    if ( !dstDir.empty() && !dstDir.EndsWith("\\") )
        dstDir += "\\";
    if ( !srcDir.empty() && !m_srcDir.EndsWith("\\") )
        srcDir += "\\";

    commands.reserve(m_dataFiles.size());
    for ( const auto& f : m_dataFiles )
    {
        command.Printf("xcopy %%1%s%s %%2%s /D /Y /Q > nul", srcDir, f, dstDir);
        commands.push_back(std::move(command));
    }

    return commands;
}

// Cannot load internat sample files,it uses catalogues
static constexpr auto forcedXCopyCommandsInternat =
R"(xcopy %1ar %2ar\ /D /Y /S /Q > nul
xcopy %1bg %2bg\ /D /Y /S /Q > nul
xcopy %1cs %2cs\ /D /Y /S /Q > nul
xcopy %1de %2de\ /D /Y /S /Q > nul
xcopy %1fr %2fr\ /D /Y /S /Q > nul
xcopy %1it %2it\ /D /Y /S /Q > nul
xcopy %1ja %2ja\ /D /Y /S /Q > nul
xcopy %1ja_JP.EUC-JP %2ja_JP.EUC-JP\ /D /Y /S /Q > nul
xcopy %1ka %2ka\ /D /Y /S /Q > nul
xcopy %1pl %2pl\ /D /Y /S /Q > nul
xcopy %1ru %2ru\ /D /Y /S /Q > nul
xcopy %1sv %2sv\ /D /Y /S /Q > nul)";

// returns true if the project file was parsed successfully
// even when there are no data files, false if there was an error
bool LoadProjectData(const wxString& path, ProjectData& data, wxArrayString& errors, size_t& warningCount)
{
    // manual hacks for projects that cannot be load
    const wxString projectFolderName = wxFileName(path).GetDirs().back();

    if ( projectFolderName == "internat" )
    {
        data.ForceXCopyCommands(wxSplit(forcedXCopyCommandsInternat, '\n', '\0'));
        return true;
    }


    wxXmlDocument doc;

    {
        wxLogNull logNo;

        if ( !doc.Load(path) )
        {
            errors.push_back("Could not load XML file.");
            return false;
        }
    }

    wxXmlNode* rootNode = doc.GetRoot();

    if ( rootNode->GetName() != "makefile" )
    {
        errors.push_back(wxString::Format("Invalid root node name: '%s'", rootNode->GetName()));
        return false;
    }

    bool hasDataNode = false;
    wxString dstDir, srcDir;
    wxString files;

    for ( wxXmlNode* rootChild = rootNode->GetChildren();
          rootChild;
          rootChild = rootChild->GetNext() )
    {
        const wxString name = rootChild->GetName();

        if ( rootChild->GetName() == "wx-data" )
        {
            if ( hasDataNode )
            {
                errors.push_back("Multiple 'wx-data' nodes not supported");
                return false;
            }

            hasDataNode = true;

            for ( wxXmlNode* dataChild = rootChild->GetChildren();
                  dataChild;
                  dataChild = dataChild->GetNext() )
            {
                const wxString nodeName = dataChild->GetName();

                if ( nodeName == "dstdir" )
                {
                    dstDir = dataChild->GetNodeContent().AfterLast('/');
                    if ( dstDir.empty() )
                        errors.push_back("Invalid dstdir");
                }
                else if ( nodeName == "srcdir" )
                {
                    srcDir = dataChild->GetNodeContent().AfterLast('/');
                    if ( srcDir.empty() )
                        errors.push_back("Invalid srcdir");
                }
                else if ( nodeName == "files" )
                {
                    files = dataChild->GetNodeContent();
                    files = files.Trim(true).Trim(false);
                    files.Replace("\n", " ");
                    while ( files.Replace("  ", " ") > 0 )
                    {
                        ;
                    }
                }
                else
                {
                    warningCount++;
                    wxLogWarning("Unknown 'wx-data' node name '%s' in '%s'", nodeName, path);
                }
            }
            break;
        }
    }

    if ( hasDataNode )
    {
        if ( files.empty() )
        {
            errors.push_back("Has wx-data node but no files found.");
            return false;
        }

        if ( srcDir.empty() != dstDir.empty() )
        {
            errors.push_back("Only one of the src/dest dir pair set.");
            return false;
        }

        data.AssignDirs(dstDir, srcDir);
        data.AssignDataFiles(wxSplit(files, ' '));
    }

    return true;
}

void LoadProjectsData(const wxArrayString& projectsPaths, ProjectDatas& datas,
                        ProjectErrors& errors, size_t& warningCount)
{
    datas.clear();
    errors.clear();
    warningCount = 0;

    for ( const auto& pp : projectsPaths )
    {
        ProjectData pd(pp);
        wxArrayString pe;

        wxLogMessage("----------\nProcessing '%s':", pp);
        if ( LoadProjectData(pp, pd, pe, warningCount) )
        {
            if ( !pd.HasFiles() && !pd.HasForcedXCopyCommands() )
            {
                wxLogMessage("  No data files found");
                continue;
            }
            if ( pd.HasFiles() )
            {
                const wxArrayString files = pd.GetFileNames();

                wxLogMessage("  Found %zu data file(s): %s", files.size(), wxJoin(files, ' ', '\0'));
            }
            if ( pd.HasForcedXCopyCommands() )
            {
                wxLogMessage("  Uses forced XCopy commands");
            }

            datas.push_back(std::move(pd));
        }
        else
        {
            wxLogMessage("  %zu errors:", pe.size());
            for ( const auto& e : pe )
                wxLogMessage("    %s", e);
            errors.push_back({pp, pe});
        }
    }
}

void WriteCmdFiles(const ProjectDatas& datas,size_t& cmdFilesWritten, ProjectErrors& errors,
                   size_t& warningCount)
{
    static constexpr auto cmdFileName("copyfiles.cmd");
    static constexpr auto cmdFileStart =
R"(@echo off

REM Copies sample data files to the folder with its executable.
REM Called from MSVS as the sample's post-build step where the first
REM argument is the project folder with data files and the second
REM the folder where the executable is built. Both folder names must
REM end with a backslash.

IF "%2" EQU "" GOTO err_args
IF "%3" NEQ "" GOTO err_args)";

    static constexpr auto cmdFileEnd =
R"(:success
exit /B 0

:err_args
echo Error: Must be called with 2 arguments, see the comments at the top of the file.
exit /B -1

:err_copy
echo Error: Could not copy all sample files.
exit /B %errorlevel%)";

    static constexpr auto checkErrorCommand = R"(IF %ERRORLEVEL% NEQ 0 GOTO err_copy)";

    static constexpr auto lineEnd{"\n"};

    errors.clear();
    warningCount = 0;
    cmdFilesWritten = 0;

    for ( const auto& d : datas )
    {
        wxLogMessage("Writing commands for '%s'...", d.GetProjectPath());

        const wxString projectPath = d.GetProjectPath();
        const wxArrayString XCopyCommands = d.GetXCopyCommands();
        wxString command;
        wxArrayString commands;
        ProjectError pe(projectPath);

        commands.reserve(XCopyCommands.size());
        for ( const auto& xcc : XCopyCommands )
        {
            command.Printf("%s\n%s\n", xcc, checkErrorCommand);
            commands.push_back(command);
        }

        const wxArrayString dataFiles = d.GetFileNames();
        const wxString srcDir = d.GetSrcDir();

        for ( const auto& f : dataFiles )
        {
            wxFileName fn(projectPath);

            if ( !srcDir.empty() )
                fn.AppendDir(srcDir);
            fn.SetFullName(f);
            if ( !fn.FileExists() )
            {
                warningCount++;
                wxLogWarning("File '%s' not found.", fn.GetFullPath());
            }
        }

        wxString fileContents;

        fileContents.Printf("%s\n\n%s\n%s", cmdFileStart,
            wxJoin(commands, '\n', '\0'), cmdFileEnd);

        wxFileName cmdFilePath(projectPath);

        cmdFilePath.SetFullName(cmdFileName);
        wxFFile f(cmdFilePath.GetFullPath(), "wb");

        if ( !f.IsOpened() )
        {
            pe.m_errors.push_back("Could not create command file.");
            continue;
        }
        if ( !f.Write(fileContents) )
        {
            pe.m_errors.push_back("Could not write command file.");
            continue;
        }
        cmdFilesWritten++;
    }
}