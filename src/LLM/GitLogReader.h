#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <array>
#include <set>
#include <map>
#include <algorithm>

// Git提交记录结构体
struct GitCommit
{
	std::string hash;
	std::string author;
	std::string email;
	std::string date;
	std::string message;
	std::string branch;

	GitCommit() = default;
	GitCommit(const std::string& h, const std::string& a, const std::string& e,
		const std::string& d, const std::string& m)
		: hash(h), author(a), email(e), date(d), message(m) {}
};

// 文件变更信息结构体
struct FileChange
{
	std::string filename;
	std::string status;      // A=added, M=modified, D=deleted, R=renamed
	std::string oldFilename; // 重命名前的文件名
	int additions = 0;
	int deletions = 0;
	std::string diffContent; // 详细diff内容
	std::string language;    // 编程语言

	FileChange() = default;
	FileChange(const std::string& name, const std::string& stat, int add = 0, int del = 0)
		: filename(name), status(stat), additions(add), deletions(del) {}

	int totalChanges() const { return additions + deletions; }
	bool isRenamed() const { return status.find('R') == 0; }
	bool isCodeFile() const;
};

// 查询参数结构体
struct GitQueryParams
{
	int limit = 10;
	std::string since;
	std::string until;
	std::string author;
	std::string branch;
	std::string filepath;
	bool includeStats = false;
	bool includeDiff = false;

	GitQueryParams& setLimit(int l) { limit = l; return *this; }
	GitQueryParams& setSince(const std::string& s) { since = s; return *this; }
	GitQueryParams& setUntil(const std::string& u) { until = u; return *this; }
	GitQueryParams& setAuthor(const std::string& a) { author = a; return *this; }
	GitQueryParams& setBranch(const std::string& b) { branch = b; return *this; }
	GitQueryParams& setFilepath(const std::string& f) { filepath = f; return *this; }
	GitQueryParams& setIncludeStats(bool s) { includeStats = s; return *this; }
	GitQueryParams& setIncludeDiff(bool d) { includeDiff = d; return *this; }
};

// 提交变更分析结果
struct CommitAnalysis
{
	std::string commitHash;
	std::string commitRange;  // 用于范围比较
	GitCommit commitInfo;
	std::vector<FileChange> changedFiles;
	std::vector<std::string> unchangedFiles;

	// 统计信息
	int totalFiles = 0;
	int totalAdditions = 0;
	int totalDeletions = 0;
	int addedFiles = 0;
	int modifiedFiles = 0;
	int deletedFiles = 0;
	int renamedFiles = 0;

	void calculateStats();
	std::vector<FileChange> getTopChangedFiles(int count = 10) const;
	std::vector<FileChange> getCodeFiles() const;
	bool isEmpty() const { return changedFiles.empty(); }
};

// Git仓库信息
struct RepoInfo
{
	std::string currentBranch;
	std::string lastCommitHash;
	bool hasUncommittedChanges = false;
	int totalCommits = 0;
	std::vector<std::string> branches;
};

// LLM代码审查数据结构
struct CodeReviewData
{
	// 基本信息
	std::string commitHash;
	std::string commitRange;
	GitCommit commitInfo;

	// 统计摘要
	int totalFilesChanged = 0;
	int totalAdditions = 0;
	int totalDeletions = 0;
	int codeFilesChanged = 0;

	// 主要变更文件（按变更量排序）
	std::vector<FileChange> mainChangedFiles;

	// 代码文件的详细diff
	std::vector<FileChange> codeFileChanges;

	// 格式化的审查文本
	std::string reviewText;
	std::string diffSummary;

	bool isEmpty() const { return mainChangedFiles.empty(); }
};

class GitLogReader
{
public:
	// 构造函数
	explicit GitLogReader(const std::string& path = ".");

	// === 基础仓库信息 ===
	RepoInfo getRepoInfo();
	std::string getCurrentBranch();
	bool isGitRepo();

	// === 提交记录查询 ===
	std::vector<GitCommit> getCommits(const GitQueryParams& params = GitQueryParams());
	std::vector<GitCommit> getCommits(int limit) { return getCommits(GitQueryParams().setLimit(limit)); }
	std::vector<GitCommit> getFileHistory(const std::string& filepath, int limit = 10);
	GitCommit getCommit(const std::string& commitHash);

	// === 变更分析 ===
	CommitAnalysis analyzeCommit(const std::string& commitHash);
	CommitAnalysis analyzeLastCommit();
	CommitAnalysis compareCommits(const std::string& fromCommit, const std::string& toCommit);
	CommitAnalysis analyzeWorkingDirectory();
	CommitAnalysis analyzeRange(const std::string& fromCommit, const std::string& toCommit = "HEAD");

	// === 代码审查支持 ===
	std::string formatForCodeReview(const CommitAnalysis& analysis, bool includeFullDiff = false);
	std::string getFileContentAtCommit(const std::string& filename, const std::string& commitHash);
	std::string getFileDiff(const std::string& filename, const std::string& fromCommit, const std::string& toCommit = "");

	// === LLM代码审查接口 ===
	CodeReviewData getCodeReviewData(const std::string& commitHash = "");
	CodeReviewData getLastCommitReviewData();
	CodeReviewData getWorkingDirectoryReviewData();
	CodeReviewData getRangeReviewData(const std::string& fromCommit, const std::string& toCommit);
	std::string formatForLLM(const CodeReviewData& reviewData, bool includeDiffDetails = true);
	std::string getSimplifiedDiff(const std::string& commitHash = "", int maxLines = 1000);

	// === 输出和报告 ===
	void printCommits(const std::vector<GitCommit>& commits);
	void printAnalysis(const CommitAnalysis& analysis);
	void generateReport(const CommitAnalysis& analysis);

	// === 设置 ===
	void setRepoPath(const std::string& path) { repoPath = path; }
	std::string getRepoPath() const { return repoPath; }
	void setVerbose(bool v) { verbose = v; }

	static const std::set<std::string> CODE_EXTENSIONS;
	static const std::map<std::string, std::string> LANGUAGE_MAP;

private:
	std::string repoPath;
	bool verbose = false;

	// === 核心Git操作 ===
	std::string executeGitCommand(const std::string& gitArgs);
	std::string buildGitLogCommand(const GitQueryParams& params);

	// === 解析器 ===
	std::vector<GitCommit> parseCommitLog(const std::string& output, bool includeStats = false);
	GitCommit parseCommitInfo(const std::string& commitHash);
	std::vector<FileChange> parseFileChanges(const std::string& statsOutput, const std::string& statusOutput,
		const std::string& diffOutput = "");
	FileChange parseFileChange(const std::string& statLine, const std::string& statusLine,
		const std::string& diffContent = "");

	// === 文件系统辅助 ===
	std::vector<std::string> getAllTrackedFiles(const std::string& commitHash = "HEAD");
	std::vector<std::string> getUntrackedFiles();

	// === LLM审查辅助方法 ===
	CodeReviewData analysisToReviewData(const CommitAnalysis& analysis);
	std::string summarizeDiffContent(const std::vector<FileChange>& files, int maxLines = 1000);
	std::string extractKeyChanges(const FileChange& file);

	// === 工具方法 ===
	std::string trim(const std::string& str);
	std::string getFileExtension(const std::string& filename);
	std::string detectLanguage(const std::string& filename);
	bool isCodeFile(const std::string& filename);
	void logError(const std::string& message, const std::exception& e);
};