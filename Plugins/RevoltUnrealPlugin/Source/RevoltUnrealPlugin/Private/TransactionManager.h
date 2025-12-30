// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Transaction record for undo/redo operations
 */
struct FTransactionRecord
{
	FDateTime Timestamp;
	FString Description;
	FString AssetPath;
	FString BackupPath;
	TArray<TSharedPtr<FJsonObject>> Changes;
	bool bCompleted;

	FTransactionRecord()
		: Timestamp(FDateTime::Now())
		, bCompleted(false)
	{
	}
};

/**
 * Manages transactions for atomic operations and undo/redo
 */
class FTransactionManager
{
public:

	/**
	 * Get singleton instance
	 */
	static FTransactionManager& Get();

	/**
	 * Begin a new transaction
	 * @param Description Description of the transaction
	 */
	void BeginTransaction(const FString& Description);

	/**
	 * End the current transaction (commit)
	 * @return True if committed successfully
	 */
	bool EndTransaction();

	/**
	 * Cancel the current transaction (rollback)
	 * @return True if cancelled successfully
	 */
	bool CancelTransaction();

	/**
	 * Check if a transaction is currently active
	 * @return True if transaction is active
	 */
	bool IsTransactionActive() const { return CurrentTransaction != nullptr; }

	/**
	 * Add a change record to the current transaction
	 * @param AssetPath Path to modified asset
	 * @param BackupPath Path to backup file
	 * @param Change JSON object describing the change
	 */
	void RecordChange(const FString& AssetPath, const FString& BackupPath, const TSharedPtr<FJsonObject>& Change);

	/**
	 * Undo the last transaction
	 * @return True if undo successful
	 */
	bool Undo();

	/**
	 * Redo the last undone transaction
	 * @return True if redo successful
	 */
	bool Redo();

	/**
	 * Get transaction history
	 * @param MaxEntries Maximum number of entries to return
	 * @return Array of transaction records
	 */
	TArray<TSharedPtr<FJsonObject>> GetTransactionHistory(int32 MaxEntries = 10);

	/**
	 * Clear all transaction history
	 */
	void ClearHistory();

private:

	/** Private constructor for singleton */
	FTransactionManager();

	/** Current active transaction */
	TSharedPtr<FTransactionRecord> CurrentTransaction;

	/** History of completed transactions */
	TArray<TSharedPtr<FTransactionRecord>> TransactionHistory;

	/** Undo stack */
	TArray<TSharedPtr<FTransactionRecord>> UndoStack;

	/** Redo stack */
	TArray<TSharedPtr<FTransactionRecord>> RedoStack;

	/** Maximum history size */
	static const int32 MaxHistorySize = 100;

	/** Maximum undo stack size */
	static const int32 MaxUndoStackSize = 10;

	/** Singleton instance */
	static FTransactionManager* Instance;
};

