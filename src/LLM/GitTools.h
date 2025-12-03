#pragma once

#include <QJsonObject>
#include <QString>
#include <memory>

#include "GitLogReader.h"

// Git tools: manage GitLogReader lifecycle internally (Option B)
class GitTools
{
public:
	// Tool entry points (keep the original function names)
	static QJsonObject codeReviewLatest(const QJsonObject& arguments);
	static QJsonObject codeReviewCommit(const QJsonObject& arguments);
	static QJsonObject codeReviewRange(const QJsonObject& arguments);
	static QJsonObject codeReviewWorkingDir(const QJsonObject& arguments);
	static QJsonObject getCommitInfo(const QJsonObject& arguments);
	static QJsonObject getRepoStatus(const QJsonObject& arguments);

private:
	static std::unique_ptr<GitLogReader> s_reader;
	static QString s_repoPath;

	static bool ensureRepo(const QString& repoPathArg);

	static QJsonObject formatReview(const CodeReviewData& reviewData, const QString& reviewType);
	static QJsonObject error(const QString& op, const QString& msg);
};


