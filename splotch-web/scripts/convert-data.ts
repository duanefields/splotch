#!/usr/bin/env node
/**
 * convert-data.ts
 * Converts the original C Splotch data files into JSON for the TypeScript engine.
 * Run with: npx tsx scripts/convert-data.ts
 */

import * as fs from 'fs';
import * as path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const SPLOTCH_DIR = path.resolve(__dirname, '../../splotch');
const OUT_DIR = path.resolve(__dirname, '../src/data');

// ── Types ────────────────────────────────────────────────────────────────────

type ClauseType = 'OR' | 'AND' | 'NOT' | 'NAME' | 'WILDCARD';

interface TemplateClause {
  type: ClauseType;
  phrase: string;
  wildcardPre?: string;
  wildcardPost?: string;
}

interface DictEntry {
  id: number;
  raw: string;
  clauses: TemplateClause[];
  priority: number;
  responses: string[];
  nextIndex: number;
}

interface MainDict {
  entries: DictEntry[];
  defaultIndex: number;
}

interface SynEntry {
  canonical: string;
  variants: string[];
}

// ── Template clause parser ────────────────────────────────────────────────────

function parseClause(token: string): TemplateClause {
  if (token === '+') {
    return { type: 'NAME', phrase: '' };
  }
  if (token.startsWith('!')) {
    return { type: 'NOT', phrase: token.slice(1).trim() };
  }
  if (token.startsWith('&')) {
    return { type: 'AND', phrase: token.slice(1).trim() };
  }
  if (token.includes(' %')) {
    const pctIdx = token.indexOf(' %');
    const pre = token.slice(0, pctIdx).trim();
    const after = token.slice(pctIdx + 2).trim(); // skip ' %'
    return {
      type: 'WILDCARD',
      phrase: token,
      wildcardPre: pre,
      wildcardPost: after,
    };
  }
  // plain OR phrase
  return { type: 'OR', phrase: token.trim() };
}

function parseTemplateLine(line: string): TemplateClause[] {
  return line
    .split(':')
    .map(t => t.trim())
    .filter(t => t.length > 0)
    .map(parseClause);
}

// ── main.dict parser ──────────────────────────────────────────────────────────

function parseMainDict(filePath: string): MainDict {
  const raw = fs.readFileSync(filePath, 'utf-8');
  const lines = raw.split('\n');

  const entries: DictEntry[] = [];
  let i = 0;
  let id = 0;

  while (i < lines.length) {
    // Skip comment/blank lines
    while (i < lines.length && (lines[i].startsWith('#') || lines[i].trim() === '')) {
      i++;
    }
    if (i >= lines.length) break;

    const templateLine = lines[i].trim();
    i++;

    // Skip priority line (also skip blank/comment lines between)
    while (i < lines.length && (lines[i].startsWith('#') || lines[i].trim() === '')) {
      i++;
    }
    if (i >= lines.length) break;

    const priorityRaw = parseInt(lines[i].trim(), 10);
    const priority = isNaN(priorityRaw) || priorityRaw === 0 ? 9 : priorityRaw;
    i++;

    // Collect responses (lines starting with *)
    const responses: string[] = [];
    while (i < lines.length && lines[i].startsWith('*')) {
      responses.push(lines[i].slice(1)); // strip leading *
      i++;
    }

    if (responses.length === 0) continue; // malformed entry, skip

    const clauses = parseTemplateLine(templateLine);
    const entry: DictEntry = {
      id,
      raw: templateLine,
      clauses,
      priority,
      responses,
      nextIndex: 0, // will be randomized at runtime
    };
    entries.push(entry);
    id++;
  }

  // Find the default/fallback entry: the last entry (zzzzzz pattern)
  const defaultIndex = entries.length - 1;

  console.log(`Parsed ${entries.length} templates from main.dict`);
  console.log(`Default (fallback) template index: ${defaultIndex}`);
  console.log(`  Template: "${entries[defaultIndex]?.raw}"`);

  return { entries, defaultIndex };
}

// ── syn.dict parser ───────────────────────────────────────────────────────────

function parseSynDict(filePath: string): SynEntry[] {
  const raw = fs.readFileSync(filePath, 'utf-8');
  const entries: SynEntry[] = [];

  for (const line of raw.split('\n')) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith('#')) continue;
    const parts = trimmed.split(':').map(p => p.trim()).filter(p => p.length > 0);
    if (parts.length < 2) continue;
    entries.push({ canonical: parts[0], variants: parts.slice(1) });
  }

  console.log(`Parsed ${entries.length} synonym entries from syn.dict`);
  return entries;
}

// ── .w file parser ────────────────────────────────────────────────────────────

function parseWordFile(filePath: string): string[] {
  const raw = fs.readFileSync(filePath, 'utf-8');
  const lines = raw.split('\n');
  const words: string[] = [];
  let skippedHeader = false;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith('#')) continue;

    // Skip the count header line (first non-comment line that is a pure integer)
    if (!skippedHeader && /^\d+$/.test(trimmed)) {
      skippedHeader = true;
      continue;
    }
    skippedHeader = true;
    words.push(trimmed);
  }

  return words;
}

// ── Main ──────────────────────────────────────────────────────────────────────

function main() {
  fs.mkdirSync(`${OUT_DIR}/words`, { recursive: true });

  // Convert main.dict
  const mainDict = parseMainDict(path.join(SPLOTCH_DIR, 'main.dict'));
  fs.writeFileSync(
    path.join(OUT_DIR, 'main-dict.json'),
    JSON.stringify(mainDict, null, 2),
  );
  console.log('Wrote src/data/main-dict.json');

  // Convert syn.dict
  const synDict = parseSynDict(path.join(SPLOTCH_DIR, 'syn.dict'));
  fs.writeFileSync(
    path.join(OUT_DIR, 'syn-dict.json'),
    JSON.stringify(synDict, null, 2),
  );
  console.log('Wrote src/data/syn-dict.json');

  // Convert all .w files
  const wordsDir = path.join(SPLOTCH_DIR, 'words');
  const wFiles = fs.readdirSync(wordsDir).filter(f => f.endsWith('.w'));
  let wordFileCount = 0;

  for (const wFile of wFiles) {
    const name = path.basename(wFile, '.w');
    const words = parseWordFile(path.join(wordsDir, wFile));
    if (words.length === 0) continue;

    fs.writeFileSync(
      path.join(OUT_DIR, 'words', `${name}.json`),
      JSON.stringify({ name, words }, null, 2),
    );
    wordFileCount++;
  }

  console.log(`Wrote ${wordFileCount} word files to src/data/words/`);
  console.log('\nData conversion complete.');
}

main();
