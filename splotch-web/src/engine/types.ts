// Shared types for the Splotch engine

export type ClauseType = 'OR' | 'AND' | 'NOT' | 'NAME' | 'WILDCARD';

export interface TemplateClause {
  type: ClauseType;
  phrase: string;
  wildcardPre?: string;  // text before % (WILDCARD only)
  wildcardPost?: string; // text after % (WILDCARD only); empty = trailing wildcard
}

export interface DictEntry {
  id: number;
  raw: string;
  clauses: TemplateClause[];
  priority: number;   // 1–9
  responses: string[];
  nextIndex: number;  // mutable cycling cursor
}

export interface MainDict {
  entries: DictEntry[];
  defaultIndex: number;
}

export interface SynEntry {
  canonical: string;
  variants: string[];
}

export interface WordFile {
  name: string;
  words: string[];
}

export interface SplotchData {
  entries: DictEntry[];
  defaultIndex: number;
  synonyms: SynEntry[];
  wordFiles: Map<string, string[]>;
}
