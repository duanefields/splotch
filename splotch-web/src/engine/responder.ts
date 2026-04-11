import type { DictEntry } from './types';
import { applyGrammarFix, applyPostGrammarFix } from './grammar';
import { expandWordFileRefs } from './wordFiles';

/**
 * Clean captured wildcard words before grammar-swapping and substitution.
 * Strips trailing clauses, bot name references, and punctuation.
 * Mirrors the cleanup code at the top of C's _sp_usetempl.
 */
function cleanCapturedWords(words: string, botName: string): string {
  let s = words;

  // Truncate at first occurrence of clause-ending punctuation
  for (const sep of [',', '. ', '...', '?', ';', '!', ':']) {
    const idx = s.indexOf(sep);
    if (idx !== -1) s = s.slice(0, idx);
  }

  // Remove " botName" unless preceded by "than "
  const botToken = ` ${botName.toLowerCase()}`;
  const thanBot = `than${botToken}`;
  const botIdx = s.toLowerCase().indexOf(botToken);
  if (botIdx !== -1 && !s.toLowerCase().slice(0, botIdx).endsWith('than')) {
    s = s.slice(0, botIdx);
  }
  void thanBot; // only used conceptually above

  // Remove " please"
  const pleaseIdx = s.toLowerCase().indexOf(' please');
  if (pleaseIdx !== -1) s = s.slice(0, pleaseIdx);

  // Strip trailing punctuation
  s = s.replace(/[.,!?;:]+$/, '');

  return s.trim();
}

/**
 * Select the next response from an entry (cycling through alternatives),
 * substitute captured words, apply grammar fixes, expand @word.w refs.
 * Mirrors C's _sp_usetempl.
 */
export function buildResponse(
  entry: DictEntry,
  capturedWords: string,
  botName: string,
  wordFiles: Map<string, string[]>,
  rng: () => number,
): string {
  // Cycle through response alternatives (wrapping at end)
  const idx = entry.nextIndex % entry.responses.length;
  entry.nextIndex = (entry.nextIndex + 1) % entry.responses.length;
  let response = entry.responses[idx];

  if (capturedWords.trim()) {
    const cleaned = cleanCapturedWords(capturedWords, botName);
    const fixed = applyGrammarFix(cleaned);

    // Substitute % in response with grammar-fixed captured words
    response = response.replace(/%/g, fixed);
  }

  // Replace bot name with "I" in response (mirrors _sp_swap(response, botName, "I"))
  const botEscaped = botName.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  response = response.replace(
    new RegExp(`(?<![a-zA-Z0-9])${botEscaped}(?![a-zA-Z0-9])`, 'gi'),
    'I',
  );

  // Clear any remaining % placeholders
  response = response.replace(/%/g, ' ');

  // Post-grammar fixes on the full response string
  response = applyPostGrammarFix(response);

  // Expand @file.w word-file references (potentially recursive)
  response = expandWordFileRefs(response, wordFiles, rng);

  return response.trim();
}
