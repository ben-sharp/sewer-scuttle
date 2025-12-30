// Copyright Epic Games, Inc. All Rights Reserved.

#include "TransactionManager.h"
#include "BackupManager.h"

FTransactionManager* FTransactionManager::Instance = nullptr;

FTransactionManager::FTransactionManager()
{
}

FTransactionManager& FTransactionManager::Get()
{
	if (!Instance)
	{
		Instance = new FTransactionManager();
	}
	return *Instance;
}

void FTransactionManager::BeginTransaction(const FString& Description)
{
	if (CurrentTransaction)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Transaction already active, ending previous transaction"));
		EndTransaction();
	}

	CurrentTransaction = MakeShared<FTransactionRecord>();
	CurrentTransaction->Description = Description;
	CurrentTransaction->Timestamp = FDateTime::Now();

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Transaction started: '%s'"), *Description);
}

bool FTransactionManager::EndTransaction()
{
	if (!CurrentTransaction)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: No active transaction to end"));
		return false;
	}

	CurrentTransaction->bCompleted = true;

	// Add to history
	TransactionHistory.Add(CurrentTransaction);
	
	// Trim history if too large
	if (TransactionHistory.Num() > MaxHistorySize)
	{
		TransactionHistory.RemoveAt(0);
	}

	// Add to undo stack
	UndoStack.Add(CurrentTransaction);
	if (UndoStack.Num() > MaxUndoStackSize)
	{
		UndoStack.RemoveAt(0);
	}

	// Clear redo stack when new transaction completes
	RedoStack.Empty();

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Transaction completed: '%s' (%d changes)"),
		*CurrentTransaction->Description, CurrentTransaction->Changes.Num());

	CurrentTransaction.Reset();
	return true;
}

bool FTransactionManager::CancelTransaction()
{
	if (!CurrentTransaction)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: No active transaction to cancel"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Transaction cancelled: '%s'"), *CurrentTransaction->Description);

	// Restore all backups from this transaction
	for (const auto& Change : CurrentTransaction->Changes)
	{
		FString BackupPath;
		if (Change->TryGetStringField(TEXT("backup_path"), BackupPath))
		{
			if (!BackupPath.IsEmpty())
			{
				FBackupManager::RestoreBackup(BackupPath);
			}
		}
	}

	CurrentTransaction.Reset();
	return true;
}

void FTransactionManager::RecordChange(const FString& AssetPath, const FString& BackupPath, const TSharedPtr<FJsonObject>& Change)
{
	if (!CurrentTransaction)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: No active transaction to record change"));
		return;
	}

	// Add backup path to change record
	if (!BackupPath.IsEmpty())
	{
		Change->SetStringField(TEXT("backup_path"), BackupPath);
	}
	Change->SetStringField(TEXT("asset_path"), AssetPath);

	CurrentTransaction->Changes.Add(Change);
}

bool FTransactionManager::Undo()
{
	if (UndoStack.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: No transactions to undo"));
		return false;
	}

	// Get last transaction
	TSharedPtr<FTransactionRecord> Transaction = UndoStack.Pop();

	// Restore all backups
	bool bSuccess = true;
	for (const auto& Change : Transaction->Changes)
	{
		FString BackupPath;
		if (Change->TryGetStringField(TEXT("backup_path"), BackupPath))
		{
			if (!FBackupManager::RestoreBackup(BackupPath))
			{
				bSuccess = false;
			}
		}
	}

	if (bSuccess)
	{
		// Move to redo stack
		RedoStack.Add(Transaction);
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Undone transaction: '%s'"), *Transaction->Description);
	}

	return bSuccess;
}

bool FTransactionManager::Redo()
{
	if (RedoStack.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: No transactions to redo"));
		return false;
	}

	// For now, log that redo would happen
	TSharedPtr<FTransactionRecord> Transaction = RedoStack.Pop();
	UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Redo transaction: '%s' - Feature requires re-executing operations"), *Transaction->Description);

	// Move back to undo stack
	UndoStack.Add(Transaction);

	return true;
}

TArray<TSharedPtr<FJsonObject>> FTransactionManager::GetTransactionHistory(int32 MaxEntries)
{
	TArray<TSharedPtr<FJsonObject>> History;

	int32 StartIndex = FMath::Max(0, TransactionHistory.Num() - MaxEntries);
	for (int32 i = StartIndex; i < TransactionHistory.Num(); i++)
	{
		const TSharedPtr<FTransactionRecord>& Record = TransactionHistory[i];
		
		TSharedPtr<FJsonObject> HistoryJson = MakeShared<FJsonObject>();
		HistoryJson->SetStringField(TEXT("description"), Record->Description);
		HistoryJson->SetStringField(TEXT("timestamp"), Record->Timestamp.ToString());
		HistoryJson->SetNumberField(TEXT("change_count"), Record->Changes.Num());
		HistoryJson->SetBoolField(TEXT("completed"), Record->bCompleted);

		History.Add(HistoryJson);
	}

	return History;
}

void FTransactionManager::ClearHistory()
{
	TransactionHistory.Empty();
	UndoStack.Empty();
	RedoStack.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Transaction history cleared"));
}

