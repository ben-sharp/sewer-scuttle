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
    protected $signature = 'app:import-content {file=content/latest.json}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Import game content definitions from a JSON file';

    /**
     * Execute the console command.
     */
    public function handle(ContentImportService $service)
    {
        $file = $this->argument('file');
        $this->info("Importing content from: {$file}");

        try {
            $results = $service->importFromFile($file);
            $this->info("Import completed successfully!");
            $this->table(['Metric', 'Count'], [
                ['Total', $results['total']],
                ['Created', $results['created']],
                ['Updated', $results['updated']],
                ['Errors', $results['errors']],
            ]);
        } catch (\Exception $e) {
            $this->error("Import failed: " . $e->getMessage());
        }
    }
}
