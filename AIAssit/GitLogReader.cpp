#include "GitLogReader.h"
#include <filesystem>

// 静态常量定义
const std::set<std::string> GitLogReader::CODE_EXTENSIONS = {
	".cpp", ".c", ".cc", ".cxx", ".h", ".hpp", ".hxx",
	".py", ".js", ".ts", ".java", ".cs", ".php", ".rb",
	".go", ".rs", ".kt", ".swift", ".scala", ".pl",
	".sh", ".bash", ".ps1", ".sql", ".r", ".m", ".mm"
};

const std::map<std::string, std::string> GitLogReader::LANGUAGE_MAP = {
	{ ".cpp", "C++" },{ ".cc", "C++" },{ ".cxx", "C++" },
	{ ".c", "C" },{ ".h", "C/C++ Header" },{ ".hpp", "C++ Header" },
	{ ".py", "Python" },{ ".js", "JavaScript" },{ ".ts", "TypeScript" },
	{ ".java", "Java" },{ ".cs", "C#" },{ ".php", "PHP" },
	{ ".rb", "Ruby" },{ ".go", "Go" },{ ".rs", "Rust" },
	{ ".kt", "Kotlin" },{ ".swift", "Swift" },{ ".scala", "Scala" },
	{ ".sh", "Shell" },{ ".bash", "Shell" },{ ".ps1", "PowerShell" },
	{ ".sql", "SQL" },{ ".html", "HTML" },{ ".css", "CSS" },
	{ ".json", "JSON" },{ ".xml", "XML" },{ ".md", "Markdown" }
};

// ===== FileChange 方法实现 =====
bool FileChange::isCodeFile() const {
	return GitLogReader::CODE_EXTENSIONS.count(
		filename.substr(filename.find_last_of('.'))
	) > 0;
}

// ===== CommitAnalysis 方法实现 =====
void CommitAnalysis::calculateStats() {
	totalAdditions = totalDeletions = 0;
	addedFiles = modifiedFiles = deletedFiles = renamedFiles = 0;

	for (const auto& file : changedFiles) {
		totalAdditions += file.additions;
		totalDeletions += file.deletions;

		if (file.status == "A") addedFiles++;
		else if (file.status == "M") modifiedFiles++;
		else if (file.status == "D") deletedFiles++;
		else if (file.isRenamed()) renamedFiles++;
	}

	totalFiles = changedFiles.size() + unchangedFiles.size();
}

std::vector<FileChange> CommitAnalysis::getTopChangedFiles(int count) const 
{
	std::vector<FileChange> sorted = changedFiles;
	std::sort(sorted.begin(), sorted.end(),
		[](const FileChange& a, const FileChange& b) {
		return a.totalChanges() > b.totalChanges();
	});

	if (sorted.size() > static_cast<size_t>(count))
	{
		sorted.resize(count);
	}
	return sorted;
}

std::vector<FileChange> CommitAnalysis::getCodeFiles() const 
{
	std::vector<FileChange> codeFiles;
	std::copy_if(changedFiles.begin(), changedFiles.end(),
		std::back_inserter(codeFiles),
		[](const FileChange& f) { return f.isCodeFile(); });
	return codeFiles;
}

// ===== GitLogReader 实现 =====
GitLogReader::GitLogReader(const std::string& path) : repoPath(path) 
{
	if (!isGitRepo())
	{
		throw std::runtime_error("不是有效的Git仓库: " + path);
	}
}

bool GitLogReader::isGitRepo() 
{
	try 
	{
		executeGitCommand("rev-parse --git-dir");
		return true;
	}
	catch (...) 
	{
		return false;
	}
}

// === 基础仓库信息 ===
RepoInfo GitLogReader::getRepoInfo()
{
	RepoInfo info;
	try
	{
		info.currentBranch = getCurrentBranch();
		info.lastCommitHash = trim(executeGitCommand("rev-parse HEAD"));

		// 检查是否有未提交的变更
		std::string status = executeGitCommand("status --porcelain");
		info.hasUncommittedChanges = !status.empty();

		// 获取总提交数
		std::string commitCount = executeGitCommand("rev-list --count HEAD");
		info.totalCommits = std::stoi(trim(commitCount));

		// 获取所有分支
		std::string branches = executeGitCommand("branch -a --format='%(refname:short)'");
		std::istringstream branchStream(branches);
		std::string branch;
		while (std::getline(branchStream, branch))
		{
			branch = trim(branch);
			if (!branch.empty())
			{
				info.branches.push_back(branch);
			}
		}

	}
	catch (const std::exception& e) 
	{
		logError("获取仓库信息失败", e);
	}

	return info;
}

std::string GitLogReader::getCurrentBranch()
{
	try 
	{
		std::string branch = executeGitCommand("branch --show-current");
		return trim(branch);
	}
	catch (const std::exception& e) 
	{
		logError("获取当前分支失败", e);
		return "";
	}
}

// === 提交记录查询 ===
std::vector<GitCommit> GitLogReader::getCommits(const GitQueryParams& params) 
{
	try 
	{
		std::string cmd = buildGitLogCommand(params);
		std::string output = executeGitCommand(cmd);
		return parseCommitLog(output, params.includeStats);
	}
	catch (const std::exception& e) 
	{
		logError("获取提交记录失败", e);
		return{};
	}
}

std::vector<GitCommit> GitLogReader::getFileHistory(const std::string& filepath, int limit)
{
	GitQueryParams params;
	params.setLimit(limit).setFilepath(filepath);
	return getCommits(params);
}

GitCommit GitLogReader::getCommit(const std::string& commitHash) 
{
	try 
	{
		return parseCommitInfo(commitHash);
	}
	catch (const std::exception& e)
	{
		logError("获取提交信息失败", e);
		return{};
	}
}

// === 变更分析 ===
CommitAnalysis GitLogReader::analyzeCommit(const std::string& commitHash) 
{
	CommitAnalysis analysis;
	analysis.commitHash = commitHash;

	try 
	{
		// 获取提交信息
		analysis.commitInfo = parseCommitInfo(commitHash);

		// 获取文件变更
		std::string statsOutput = executeGitCommand("diff-tree --no-commit-id --numstat -r " + commitHash);
		std::string statusOutput = executeGitCommand("diff-tree --no-commit-id --name-status -r " + commitHash);
		std::string diffOutput = executeGitCommand("show --no-merges --format= " + commitHash);

		analysis.changedFiles = parseFileChanges(statsOutput, statusOutput, diffOutput);

		// 获取未改变的文件
		analysis.unchangedFiles = getAllTrackedFiles(commitHash);

		// 移除已改变的文件
		for (const auto& changed : analysis.changedFiles) 
		{
			analysis.unchangedFiles.erase(
				std::remove(analysis.unchangedFiles.begin(), analysis.unchangedFiles.end(), changed.filename),
				analysis.unchangedFiles.end()
			);
		}
		analysis.calculateStats();
	}
	catch (const std::exception& e) 
	{
		logError("分析提交失败", e);
	}

	return analysis;
}

CommitAnalysis GitLogReader::analyzeLastCommit()
{
	try 
	{
		std::string lastCommit = trim(executeGitCommand("rev-parse HEAD"));
		return analyzeCommit(lastCommit);
	}
	catch (const std::exception& e) 
	{
		logError("分析最后提交失败", e);
		return{};
	}
}

CommitAnalysis GitLogReader::compareCommits(const std::string& fromCommit, const std::string& toCommit) 
{
	CommitAnalysis analysis;
	analysis.commitRange = fromCommit + ".." + toCommit;

	try 
	{
		// 获取提交范围信息
		std::string rangeInfo = executeGitCommand("log --oneline " + fromCommit + ".." + toCommit);
		analysis.commitInfo.message = "Range: " + analysis.commitRange + "\n" + rangeInfo;
		analysis.commitInfo.author = "Multiple authors";

		// 获取差异
		std::string statsOutput = executeGitCommand("diff --numstat " + fromCommit + " " + toCommit);
		std::string statusOutput = executeGitCommand("diff --name-status " + fromCommit + " " + toCommit);
		std::string diffOutput = executeGitCommand("diff " + fromCommit + " " + toCommit);

		analysis.changedFiles = parseFileChanges(statsOutput, statusOutput, diffOutput);
		analysis.unchangedFiles = getAllTrackedFiles(toCommit);

		// 移除已改变的文件
		for (const auto& changed : analysis.changedFiles) 
		{
			analysis.unchangedFiles.erase(
				std::remove(analysis.unchangedFiles.begin(), analysis.unchangedFiles.end(), changed.filename),
				analysis.unchangedFiles.end()
			);
		}
		analysis.calculateStats();
	}
	catch (const std::exception& e) 
	{
		logError("比较提交失败", e);
	}
	return analysis;
}

CommitAnalysis GitLogReader::analyzeWorkingDirectory() 
{
	CommitAnalysis analysis;
	analysis.commitHash = "Working Directory";

	try
	{
		// 获取暂存区变化
		std::string stagedStats = executeGitCommand("diff --cached --numstat");
		std::string stagedStatus = executeGitCommand("diff --cached --name-status");

		// 获取工作区变化
		std::string unstagedStats = executeGitCommand("diff --numstat");
		std::string unstagedStatus = executeGitCommand("diff --name-status");

		// 获取未跟踪文件
		std::string untrackedOutput = executeGitCommand("ls-files --others --exclude-standard");

		// 合并解析结果
		auto stagedFiles = parseFileChanges(stagedStats, stagedStatus);
		auto unstagedFiles = parseFileChanges(unstagedStats, unstagedStatus);

		// 添加状态标记
		for (auto& file : stagedFiles) 
		{
			file.status += " (staged)";
		}
		for (auto& file : unstagedFiles)
		{
			file.status += " (unstaged)";
		}

		// 合并文件列表
		analysis.changedFiles = stagedFiles;
		analysis.changedFiles.insert(analysis.changedFiles.end(), unstagedFiles.begin(), unstagedFiles.end());

		// 处理未跟踪文件
		std::istringstream untrackedStream(untrackedOutput);
		std::string filename;
		while (std::getline(untrackedStream, filename))
		{
			filename = trim(filename);
			if (!filename.empty()) 
			{
				FileChange untracked(filename, "A (untracked)");
				untracked.language = detectLanguage(filename);
				analysis.changedFiles.push_back(untracked);
			}
		}
		analysis.unchangedFiles = getAllTrackedFiles();
		analysis.calculateStats();
	}
	catch (const std::exception& e) 
	{
		logError("分析工作目录失败", e);
	}
	return analysis;
}

CommitAnalysis GitLogReader::analyzeRange(const std::string& fromCommit, const std::string& toCommit) 
{
	return compareCommits(fromCommit, toCommit);
}

// === 代码审查支持 ===
std::string GitLogReader::formatForCodeReview(const CommitAnalysis& analysis, bool includeFullDiff)
{
	std::ostringstream formatted;

	formatted << "=== 代码审查报告 ===\n";
	formatted << "提交: " << analysis.commitHash << analysis.commitRange << "\n";
	formatted << "作者: " << analysis.commitInfo.author << "\n";
	formatted << "日期: " << analysis.commitInfo.date << "\n";
	formatted << "信息: " << analysis.commitInfo.message << "\n\n";

	formatted << "=== 变更统计 ===\n";
	formatted << "文件总数: " << analysis.totalFiles << "\n";
	formatted << "变更文件: " << analysis.changedFiles.size() << "\n";
	formatted << "新增行数: +" << analysis.totalAdditions << "\n";
	formatted << "删除行数: -" << analysis.totalDeletions << "\n";
	formatted << "净变化: " << (analysis.totalAdditions - analysis.totalDeletions) << "\n\n";

	formatted << "=== 文件变更 ===\n";
	formatted << "新增: " << analysis.addedFiles << " | ";
	formatted << "修改: " << analysis.modifiedFiles << " | ";
	formatted << "删除: " << analysis.deletedFiles << " | ";
	formatted << "重命名: " << analysis.renamedFiles << "\n\n";

	// 显示变更最大的文件
	auto topFiles = analysis.getTopChangedFiles(10);
	if (!topFiles.empty()) 
	{
		formatted << "=== 主要变更文件 ===\n";
		for (const auto& file : topFiles) 
		{
			formatted << file.filename << " [" << file.language << "] ";
			formatted << "(+" << file.additions << "/-" << file.deletions << ")\n";
		}
		formatted << "\n";
	}

	// 代码文件详情
	auto codeFiles = analysis.getCodeFiles();
	if (!codeFiles.empty())
	{
		formatted << "=== 代码文件变更详情 ===\n";
		for (const auto& file : codeFiles) 
		{
			formatted << "\n--- " << file.filename << " [" << file.language << "] ---\n";
			formatted << "状态: " << file.status << " (+" << file.additions << "/-" << file.deletions << ")\n";

			if (includeFullDiff && !file.diffContent.empty()) {
				formatted << "\n" << file.diffContent << "\n";
			}
		}
	}

	return formatted.str();
}

std::string GitLogReader::getFileContentAtCommit(const std::string& filename, const std::string& commitHash) {
	try
	{
		return executeGitCommand("show " + commitHash + ":\"" + filename + "\"");
	}
	catch (const std::exception& e) 
	{
		logError("获取文件内容失败", e);
		return "";
	}
}

std::string GitLogReader::getFileDiff(const std::string& filename, const std::string& fromCommit, const std::string& toCommit) {
	try 
	{
		std::string cmd = "diff ";
		if (!toCommit.empty()) 
		{
			cmd += fromCommit + " " + toCommit;
		}
		else 
		{
			cmd += fromCommit;
		}
		cmd += " -- \"" + filename + "\"";
		return executeGitCommand(cmd);
	}
	catch (const std::exception& e)
	{
		logError("获取文件差异失败", e);
		return "";
	}
}

// === 输出和报告 ===
void GitLogReader::printCommits(const std::vector<GitCommit>& commits)
{
	std::cout << "\n=== Git 提交记录 ===\n";
	std::cout << std::string(80, '-') << "\n";

	for (const auto& commit : commits) 
	{
		std::cout << "哈希: " << commit.hash.substr(0, 8) << "\n";
		std::cout << "作者: " << commit.author;
		if (!commit.email.empty()) 
		{
			std::cout << " <" << commit.email << ">";
		}
		std::cout << "\n";
		std::cout << "日期: " << commit.date << "\n";
		std::cout << "信息: " << commit.message << "\n";
		std::cout << std::string(80, '-') << "\n";
	}
}

void GitLogReader::printAnalysis(const CommitAnalysis& analysis)
{
	std::cout << formatForCodeReview(analysis, false);
}

void GitLogReader::generateReport(const CommitAnalysis& analysis)
{
	std::cout << formatForCodeReview(analysis, true);
}

// === 核心Git操作 ===
std::string GitLogReader::executeGitCommand(const std::string& gitArgs) 
{
	std::string cmd = "cd \"" + repoPath + "\" && git " + gitArgs;

	if (verbose) 
	{
		std::cout << "执行命令: " << cmd << std::endl;
	}

	std::array<char, 128> buffer;
	std::string result;

	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
	if (!pipe) 
	{
		throw std::runtime_error("执行Git命令失败: " + gitArgs);
	}

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) 
	{
		result += buffer.data();
	}

	return result;
}

std::string GitLogReader::buildGitLogCommand(const GitQueryParams& params) 
{
	std::string cmd = "log --pretty=format:\"%H|%an|%ae|%ad|%s\" --date=iso";

	if (params.limit > 0)
	{
		cmd += " -n " + std::to_string(params.limit);
	}

	if (!params.since.empty()) 
	{
		cmd += " --since=\"" + params.since + "\"";
	}

	if (!params.until.empty())
	{
		cmd += " --until=\"" + params.until + "\"";
	}

	if (!params.author.empty()) 
	{
		cmd += " --author=\"" + params.author + "\"";
	}

	if (!params.branch.empty())
	{
		cmd += " " + params.branch;
	}

	if (!params.filepath.empty()) 
	{
		cmd += " -- \"" + params.filepath + "\"";
	}

	return cmd;
}

// === 解析器 ===
std::vector<GitCommit> GitLogReader::parseCommitLog(const std::string& output, bool includeStats)
{
	std::vector<GitCommit> commits;
	std::istringstream stream(output);
	std::string line;

	while (std::getline(stream, line)) 
	{
		if (line.empty()) continue;

		std::vector<std::string> parts;
		std::stringstream ss(line);
		std::string part;

		while (std::getline(ss, part, '|'))
		{
			parts.push_back(part);
		}

		if (parts.size() >= 5) 
		{
			GitCommit commit(parts[0], parts[1], parts[2], parts[3], parts[4]);
			commits.push_back(commit);
		}
	}

	return commits;
}

GitCommit GitLogReader::parseCommitInfo(const std::string& commitHash) 
{
	std::string output = executeGitCommand("log -1 --pretty=format:\"%H|%an|%ae|%ad|%s\" --date=iso " + commitHash);

	std::vector<std::string> parts;
	std::stringstream ss(output);
	std::string part;

	while (std::getline(ss, part, '|')) 
	{
		parts.push_back(part);
	}

	if (parts.size() >= 5) 
	{
		return GitCommit(parts[0], parts[1], parts[2], parts[3], parts[4]);
	}

	return{};
}

std::vector<FileChange> GitLogReader::parseFileChanges(const std::string& statsOutput,
	const std::string& statusOutput,
	const std::string& diffOutput)
{
	std::vector<FileChange> changes;

	// 构建状态映射
	std::map<std::string, std::string> statusMap;
	std::istringstream statusStream(statusOutput);
	std::string line;

	while (std::getline(statusStream, line)) 
	{
		if (!line.empty())
		{
			size_t tabPos = line.find('\t');
			if (tabPos != std::string::npos) 
			{
				std::string status = line.substr(0, tabPos);
				std::string filename = line.substr(tabPos + 1);
				statusMap[filename] = status;
			}
		}
	}

	// 解析统计信息
	std::istringstream statsStream(statsOutput);
	while (std::getline(statsStream, line))
	{
		if (!line.empty()) 
		{
			std::istringstream lineStream(line);
			std::string adds, dels, filename;

			lineStream >> adds >> dels;
			std::getline(lineStream, filename);
			filename = trim(filename);

			if (!filename.empty()) 
			{
				FileChange change;
				change.filename = filename;
				change.additions = (adds == "-") ? 0 : std::stoi(adds);
				change.deletions = (dels == "-") ? 0 : std::stoi(dels);
				change.status = statusMap[filename];
				change.language = detectLanguage(filename);

				// 处理重命名
				if (change.status.find('R') == 0)
				{
					size_t arrowPos = filename.find(" -> ");
					if (arrowPos != std::string::npos)
					{
						change.oldFilename = filename.substr(0, arrowPos);
						change.filename = filename.substr(arrowPos + 4);
					}
				}

				// 如果有diff内容，尝试提取该文件的部分
				if (!diffOutput.empty()) 
				{
					// 简单的diff提取 - 可以进一步优化
					size_t diffStart = diffOutput.find("diff --git a/" + change.filename);
					if (diffStart != std::string::npos) 
					{
						size_t diffEnd = diffOutput.find("diff --git", diffStart + 1);
						if (diffEnd == std::string::npos)
						{
							diffEnd = diffOutput.length();
						}
						change.diffContent = diffOutput.substr(diffStart, diffEnd - diffStart);
					}
				}
				changes.push_back(change);
			}
		}
	}
	return changes;
}

// === 文件系统辅助 ===
std::vector<std::string> GitLogReader::getAllTrackedFiles(const std::string& commitHash) 
{
	std::vector<std::string> files;
	try
	{
		std::string cmd = "ls-tree -r --name-only " + commitHash;
		std::string output = executeGitCommand(cmd);

		std::istringstream stream(output);
		std::string filename;
		while (std::getline(stream, filename)) 
		{
			filename = trim(filename);
			if (!filename.empty()) 
			{
				files.push_back(filename);
			}
		}
	}
	catch (const std::exception& e) 
	{
		logError("获取跟踪文件失败", e);
	}

	return files;
}

std::vector<std::string> GitLogReader::getUntrackedFiles()
{
	std::vector<std::string> files;
	try 
	{
		std::string output = executeGitCommand("ls-files --others --exclude-standard");
		std::istringstream stream(output);
		std::string filename;
		while (std::getline(stream, filename)) 
		{
			filename = trim(filename);
			if (!filename.empty()) 
			{
				files.push_back(filename);
			}
		}
	}
	catch (const std::exception& e) 
	{
		logError("获取未跟踪文件失败", e);
	}
	return files;
}

// === 工具方法 ===
std::string GitLogReader::trim(const std::string& str) 
{
	size_t first = str.find_first_not_of(" \t\n\r");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\n\r");
	return str.substr(first, (last - first + 1));
}

std::string GitLogReader::getFileExtension(const std::string& filename)
{
	size_t lastDot = filename.find_last_of('.');
	if (lastDot == std::string::npos) 
	{
		return "";
	}
	return filename.substr(lastDot);
}

std::string GitLogReader::detectLanguage(const std::string& filename)
{
	std::string ext = getFileExtension(filename);
	auto it = LANGUAGE_MAP.find(ext);
	return (it != LANGUAGE_MAP.end()) ? it->second : "Unknown";
}

bool GitLogReader::isCodeFile(const std::string& filename) 
{
	return CODE_EXTENSIONS.count(getFileExtension(filename)) > 0;
}

void GitLogReader::logError(const std::string& message, const std::exception& e) 
{
	if (verbose) {
		std::cerr << "错误: " << message << " - " << e.what() << std::endl;
	}
}

// === LLM代码审查接口实现 ===
CodeReviewData GitLogReader::getCodeReviewData(const std::string& commitHash) 
{
	try 
	{
		std::string targetCommit = commitHash.empty() ? trim(executeGitCommand("rev-parse HEAD")) : commitHash;
		CommitAnalysis analysis = analyzeCommit(targetCommit);
		return analysisToReviewData(analysis);
	}
	catch (const std::exception& e) 
	{
		logError("获取代码审查数据失败", e);
		return{};
	}
}

CodeReviewData GitLogReader::getLastCommitReviewData()
{
	try 
	{
		CommitAnalysis analysis = analyzeLastCommit();
		return analysisToReviewData(analysis);
	}
	catch (const std::exception& e) 
	{
		logError("获取最后提交审查数据失败", e);
		return{};
	}
}

CodeReviewData GitLogReader::getWorkingDirectoryReviewData() 
{
	try 
	{
		CommitAnalysis analysis = analyzeWorkingDirectory();
		return analysisToReviewData(analysis);
	}
	catch (const std::exception& e) 
	{
		logError("获取工作目录审查数据失败", e);
		return{};
	}
}

CodeReviewData GitLogReader::getRangeReviewData(const std::string& fromCommit, const std::string& toCommit) 
{
	try
	{
		CommitAnalysis analysis = compareCommits(fromCommit, toCommit);
		return analysisToReviewData(analysis);
	}
	catch (const std::exception& e) 
	{
		logError("获取范围审查数据失败", e);
		return{};
	}
}

std::string GitLogReader::formatForLLM(const CodeReviewData& reviewData, bool includeDiffDetails) 
{
	if (reviewData.isEmpty())
	{
		return "无代码变更需要审查。";
	}

	std::ostringstream llmText;

	// 基本信息
	llmText << "=== 代码审查请求 ===\n";
	llmText << "提交: " << reviewData.commitHash << reviewData.commitRange << "\n";
	llmText << "作者: " << reviewData.commitInfo.author << "\n";
	llmText << "时间: " << reviewData.commitInfo.date << "\n";
	llmText << "提交信息: " << reviewData.commitInfo.message << "\n\n";

	// 变更摘要
	llmText << "=== 变更摘要 ===\n";
	llmText << "总变更文件: " << reviewData.totalFilesChanged << "\n";
	llmText << "代码文件变更: " << reviewData.codeFilesChanged << "\n";
	llmText << "新增行数: +" << reviewData.totalAdditions << "\n";
	llmText << "删除行数: -" << reviewData.totalDeletions << "\n";
	llmText << "净变化: " << (reviewData.totalAdditions - reviewData.totalDeletions) << "\n\n";

	// 主要变更文件
	if (!reviewData.mainChangedFiles.empty())
	{
		llmText << "=== 主要变更文件 ===\n";
		for (const auto& file : reviewData.mainChangedFiles)
		{
			llmText << "- " << file.filename << " [" << file.language << "] ";
			llmText << "(" << file.status << ", +" << file.additions << "/-" << file.deletions << ")\n";
		}
		llmText << "\n";
	}

	// 代码变更详情
	if (includeDiffDetails && !reviewData.codeFileChanges.empty()) 
	{
		llmText << "=== 代码变更详情 ===\n";
		llmText << "请审查以下代码变更，关注代码质量、安全性、性能和最佳实践：\n\n";

		for (const auto& file : reviewData.codeFileChanges) 
		{
			llmText << "--- " << file.filename << " [" << file.language << "] ---\n";
			llmText << "变更类型: " << file.status << " (+" << file.additions << "/-" << file.deletions << ")\n";

			if (!file.diffContent.empty())
			{
				// 提取关键变更部分
				std::string keyChanges = extractKeyChanges(file);
				if (!keyChanges.empty()) 
				{
					llmText << keyChanges << "\n";
				}
			}
			llmText << "\n";
		}
	}

	// 审查指导
	llmText << "=== 审查要点 ===\n";
	llmText << "请从以下角度进行代码审查：\n";
	llmText << "1. 代码规范性和可读性\n";
	llmText << "2. 潜在的安全漏洞\n";
	llmText << "3. 性能优化建议\n";
	llmText << "4. 错误处理和边界情况\n";
	llmText << "5. 代码重构和改进建议\n";
	llmText << "6. 测试覆盖度考虑\n\n";

	return llmText.str();
}

std::string GitLogReader::getSimplifiedDiff(const std::string& commitHash, int maxLines) 
{
	try 
	{
		std::string targetCommit = commitHash.empty() ? "HEAD" : commitHash;
		std::string diffOutput = executeGitCommand("show --format= " + targetCommit);

		// 简化diff内容，移除二进制文件和大文件
		return summarizeDiffContent({}, maxLines);
	}
	catch (const std::exception& e) 
	{
		logError("获取简化差异失败", e);
		return "";
	}
}

// === LLM审查辅助方法实现 ===
CodeReviewData GitLogReader::analysisToReviewData(const CommitAnalysis& analysis) 
{
	CodeReviewData reviewData;

	// 基本信息
	reviewData.commitHash = analysis.commitHash;
	reviewData.commitRange = analysis.commitRange;
	reviewData.commitInfo = analysis.commitInfo;

	// 统计信息
	reviewData.totalFilesChanged = analysis.changedFiles.size();
	reviewData.totalAdditions = analysis.totalAdditions;
	reviewData.totalDeletions = analysis.totalDeletions;

	// 获取代码文件
	reviewData.codeFileChanges = analysis.getCodeFiles();
	reviewData.codeFilesChanged = reviewData.codeFileChanges.size();

	// 主要变更文件（最多10个）
	reviewData.mainChangedFiles = analysis.getTopChangedFiles(10);

	// 生成摘要文本
	reviewData.diffSummary = summarizeDiffContent(reviewData.codeFileChanges);
	reviewData.reviewText = formatForLLM(reviewData, false);

	return reviewData;
}

std::string GitLogReader::summarizeDiffContent(const std::vector<FileChange>& files, int maxLines)
{
	std::ostringstream summary;
	int currentLines = 0;
	for (const auto& file : files) 
	{
		if (currentLines >= maxLines) 
		{
			summary << "\n... (输出截断，超过最大行数限制) ...\n";
			break;
		}
		if (!file.diffContent.empty())
		{
			// 分割diff内容为行
			std::istringstream diffStream(file.diffContent);
			std::string line;
			int fileLines = 0;
			summary << "=== " << file.filename << " ===\n";
			while (std::getline(diffStream, line) && currentLines < maxLines && fileLines < 50) 
			{
				// 过滤掉一些冗余信息
				if (line.find("diff --git") != 0 &&
					line.find("index ") != 0 &&
					line.find("@@") != std::string::npos)
				{
					continue;
				}
				summary << line << "\n";
				currentLines++;
				fileLines++;
			}
			if (fileLines >= 50) 
			{
				summary << "... (文件内容截断) ...\n";
			}
			summary << "\n";
		}
	}
	return summary.str();
}

std::string GitLogReader::extractKeyChanges(const FileChange& file)
{
	if (file.diffContent.empty())
	{
		return "";
	}
	std::ostringstream keyChanges;
	std::istringstream diffStream(file.diffContent);
	std::string line;
	bool inHunk = false;
	int contextLines = 0;
	while (std::getline(diffStream, line)) 
	{
		// 识别hunk标题行
		if (line.find("@@") != std::string::npos) 
		{
			keyChanges << line << "\n";
			inHunk = true;
			contextLines = 0;
			continue;
		}
		if (inHunk) 
		{
			// 只包含真正的变更行和少量上下文
			if (line.empty()) 
			{
				continue;
			}
			char firstChar = line[0];
			if (firstChar == '+' || firstChar == '-')
			{
				keyChanges << line << "\n";
				contextLines = 0;
			}
			else if (firstChar == ' ' && contextLines < 2) 
			{
				keyChanges << line << "\n";
				contextLines++;
			}
			// 限制输出长度
			if (keyChanges.tellp() > 2000) 
			{
				keyChanges << "... (diff内容截断) ...\n";
				break;
			}
		}
	}
	return keyChanges.str();
}