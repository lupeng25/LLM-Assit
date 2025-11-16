#include "GitTools.h"
#include <QJsonArray>
#include <QDateTime>

std::unique_ptr<GitLogReader> GitTools::s_reader;
QString GitTools::s_repoPath;

bool GitTools::ensureRepo(const QString& repoPathArg)
{
	QString repo = repoPathArg;
	// Tolerance: remove accidental "think" text if present
	repo.replace("think", "");

	if (!s_reader || s_repoPath != repo)
	{
		s_repoPath = repo;
		s_reader = std::make_unique<GitLogReader>(repo.isEmpty() ? std::string("") : repo.toStdString());
	}
	return static_cast<bool>(s_reader);
}

QJsonObject GitTools::error(const QString& op, const QString& msg)
{
	QJsonObject r;
	r["success"] = false;
	r["operation"] = op;
	r["error"] = msg;
	return r;
}

QJsonObject GitTools::formatReview(const CodeReviewData& reviewData, const QString& reviewType)
{
	QJsonObject result;
	result["success"] = true;
	result["review_type"] = reviewType;
	result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

	// Commit info
	QJsonObject commitInfo;
	commitInfo["hash"] = QString::fromStdString(reviewData.commitHash);
	commitInfo["range"] = QString::fromStdString(reviewData.commitRange);
	commitInfo["author"] = QString::fromStdString(reviewData.commitInfo.author);
	commitInfo["email"] = QString::fromStdString(reviewData.commitInfo.email);
	commitInfo["date"] = QString::fromStdString(reviewData.commitInfo.date);
	commitInfo["message"] = QString::fromStdString(reviewData.commitInfo.message);
	result["commit_info"] = commitInfo;

	// Statistics
	QJsonObject stats;
	stats["total_files_changed"] = reviewData.totalFilesChanged;
	stats["code_files_changed"] = reviewData.codeFilesChanged;
	stats["total_additions"] = reviewData.totalAdditions;
	stats["total_deletions"] = reviewData.totalDeletions;
	stats["net_changes"] = reviewData.totalAdditions - reviewData.totalDeletions;
	result["statistics"] = stats;

	// Main changed files
	QJsonArray mainFiles;
	for (const auto& file : reviewData.mainChangedFiles)
	{
		QJsonObject f;
		f["filename"] = QString::fromStdString(file.filename);
		f["language"] = QString::fromStdString(file.language);
		f["status"] = QString::fromStdString(file.status);
		f["additions"] = file.additions;
		f["deletions"] = file.deletions;
		f["total_changes"] = file.totalChanges();
		mainFiles.append(f);
	}
	result["main_changed_files"] = mainFiles;

	// LLM review text
	result["llm_review_text"] = QString::fromStdString(reviewData.reviewText);
	return result;
}

QJsonObject GitTools::codeReviewLatest(const QJsonObject& arguments)
{
	try {
		if (!ensureRepo(arguments.value("repo_path").toString()))
			return error("CodeReviewLatest", "Git reader not initialized");

		CodeReviewData data = s_reader->getLastCommitReviewData();
		if (data.isEmpty())
			return error("CodeReviewLatest", "No code changes found to review");
		return formatReview(data, QStringLiteral("latest_commit"));
	} catch (const std::exception& e) {
		return error("CodeReviewLatest", e.what());
	}
}

QJsonObject GitTools::codeReviewCommit(const QJsonObject& arguments)
{
	try {
		if (!ensureRepo(arguments.value("repo_path").toString()))
			return error("CodeReviewCommit", "Git reader not initialized");
		QString commitHash = arguments.value("commit_hash").toString();
		if (commitHash.isEmpty())
			return error("CodeReviewCommit", "Commit hash is required");
		CodeReviewData data = s_reader->getCodeReviewData(commitHash.toStdString());
		if (data.isEmpty())
			return error("CodeReviewCommit", QString("No code changes found for commit %1").arg(commitHash));
		return formatReview(data, QStringLiteral("specific_commit"));
	} catch (const std::exception& e) {
		return error("CodeReviewCommit", e.what());
	}
}

QJsonObject GitTools::codeReviewRange(const QJsonObject& arguments)
{
	try {
		if (!ensureRepo(arguments.value("repo_path").toString()))
			return error("CodeReviewRange", "Git reader not initialized");
		QString from = arguments.value("from_commit").toString();
		QString to = arguments.value("to_commit").toString();
		if (from.isEmpty() || to.isEmpty())
			return error("CodeReviewRange", "Both from_commit and to_commit are required");
		CodeReviewData data = s_reader->getRangeReviewData(from.toStdString(), to.toStdString());
		if (data.isEmpty())
			return error("CodeReviewRange", QString("No code changes found for range %1..%2").arg(from, to));
		return formatReview(data, QStringLiteral("commit_range"));
	} catch (const std::exception& e) {
		return error("CodeReviewRange", e.what());
	}
}

QJsonObject GitTools::codeReviewWorkingDir(const QJsonObject& arguments)
{
	try {
		if (!ensureRepo(arguments.value("repo_path").toString()))
			return error("CodeReviewWorkingDir", "Git reader not initialized");
		CodeReviewData data = s_reader->getWorkingDirectoryReviewData();
		if (data.isEmpty())
			return error("CodeReviewWorkingDir", "No code changes found in working directory");
		return formatReview(data, QStringLiteral("working_directory"));
	} catch (const std::exception& e) {
		return error("CodeReviewWorkingDir", e.what());
	}
}

QJsonObject GitTools::getCommitInfo(const QJsonObject& arguments)
{
	try {
		if (!ensureRepo(arguments.value("repo_path").toString()))
			return error("GetCommitInfo", "Git reader not initialized");
		QString commitHash = arguments.value("commit_hash").toString();
		GitCommit commit;
		if (commitHash.isEmpty()) {
			auto commits = s_reader->getCommits(1);
			if (!commits.empty()) commit = commits[0];
		} else {
			commit = s_reader->getCommit(commitHash.toStdString());
		}
		QJsonObject r;
		r["success"] = true;
		r["commit_hash"] = QString::fromStdString(commit.hash);
		r["author"] = QString::fromStdString(commit.author);
		r["email"] = QString::fromStdString(commit.email);
		r["date"] = QString::fromStdString(commit.date);
		r["message"] = QString::fromStdString(commit.message);
		r["branch"] = QString::fromStdString(commit.branch);
		return r;
	} catch (const std::exception& e) {
		return error("GetCommitInfo", e.what());
	}
}

QJsonObject GitTools::getRepoStatus(const QJsonObject& arguments)
{
	try {
		if (!ensureRepo(arguments.value("repo_path").toString()))
			return error("GetRepoStatus", "Git reader not initialized");
		RepoInfo info = s_reader->getRepoInfo();
		QJsonObject r;
		r["success"] = true;
		r["current_branch"] = QString::fromStdString(info.currentBranch);
		r["last_commit_hash"] = QString::fromStdString(info.lastCommitHash);
		r["has_uncommitted_changes"] = info.hasUncommittedChanges;
		r["total_commits"] = info.totalCommits;
		r["repo_path"] = s_repoPath;

		QJsonArray branches;
		for (const auto& b : info.branches) branches.append(QString::fromStdString(b));
		r["branches"] = branches;
		return r;
	} catch (const std::exception& e) {
		return error("GetRepoStatus", e.what());
	}
}


