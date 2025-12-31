<?php

namespace App\Console\Commands;

use App\Services\ContentImportService;
use Illuminate\Console\Command;

class ContentImportCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'content:import {file : Path to the content JSON file}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Import content definitions from a JSON file';

    /**
     * Execute the console command.
     */
    public function handle(ContentImportService $importService): int
    {
        $filePath = $this->argument('file');

        // Resolve relative paths
        if (!str_starts_with($filePath, '/') && !str_starts_with($filePath, '\\')) {
            $filePath = base_path($filePath);
        }

        $this->info("Importing content from: {$filePath}");

        try {
            $contentVersion = $importService->importFromFile($filePath);

            $this->info("Content imported successfully!");
            $this->info("Version: {$contentVersion->version}");
            $this->info("Definitions: " . $contentVersion->definitions()->count());
            $this->info("Active: " . ($contentVersion->is_active ? 'Yes' : 'No'));

            return Command::SUCCESS;
        } catch (\Exception $e) {
            $this->error("Failed to import content: " . $e->getMessage());
            return Command::FAILURE;
        }
    }
}
