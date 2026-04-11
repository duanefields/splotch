import type { SplotchData, WordFile } from './types';
import mainDictRaw from '../data/main-dict.json';
import synDictRaw from '../data/syn-dict.json';

// Vite resolves this glob at build time into all words/*.json imports
const wordModules = import.meta.glob<{ default: WordFile }>(
  '../data/words/*.json',
  { eager: true },
);

export function loadSplotchData(): SplotchData {
  const wordFiles = new Map<string, string[]>();
  for (const [_path, mod] of Object.entries(wordModules)) {
    wordFiles.set(mod.default.name, mod.default.words);
  }

  return {
    entries: mainDictRaw.entries as SplotchData['entries'],
    defaultIndex: mainDictRaw.defaultIndex,
    synonyms: synDictRaw,
    wordFiles,
  };
}
