import type { SynEntry } from './types';

/**
 * Replace all word-boundary occurrences of `from` with `to` in `text`.
 * Mirrors C's _sp_swap which checks isspace/ispunct before and after matches.
 */
function wordBoundaryReplace(text: string, from: string, to: string): string {
  const escaped = from.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const re = new RegExp(`(?<![a-zA-Z0-9])${escaped}(?![a-zA-Z0-9])`, 'gi');
  return text.replace(re, to);
}

/**
 * Expand synonyms/contractions in user input, mirrors _sp_expand.
 * Also lowercases the input.
 */
export function expandSynonyms(text: string, synonyms: SynEntry[]): string {
  let result = text.toLowerCase();
  for (const entry of synonyms) {
    for (const variant of entry.variants) {
      result = wordBoundaryReplace(result, variant.toLowerCase(), entry.canonical.toLowerCase());
    }
  }
  return result;
}
