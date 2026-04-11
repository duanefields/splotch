/**
 * Grammar and pronoun swapping, porting C's _sp_fix / _sp_gswap.
 *
 * The C code uses a "parity bit" trick (char + 128) to mark already-swapped
 * tokens so they don't get swapped again by a later pass. We replicate this
 * with a token array where each token carries a `locked` flag.
 */

interface Token {
  text: string;
  locked: boolean;
  isWord: boolean; // false = punctuation/space gap
}

/** Split text into alternating word and gap tokens. */
function tokenize(text: string): Token[] {
  const tokens: Token[] = [];
  const re = /([a-zA-Z0-9]+)|([^a-zA-Z0-9]+)/g;
  let m: RegExpExecArray | null;
  while ((m = re.exec(text)) !== null) {
    if (m[1] !== undefined) {
      tokens.push({ text: m[1], locked: false, isWord: true });
    } else {
      tokens.push({ text: m[2], locked: false, isWord: false });
    }
  }
  return tokens;
}

function detokenize(tokens: Token[]): string {
  return tokens.map(t => t.text).join('');
}

/**
 * Apply a single swap pass over word tokens.
 * Handles single-word and two-word swaps (e.g. "that you" → "that I").
 * Matched tokens are marked locked so subsequent passes skip them.
 */
function applySwap(tokens: Token[], from: string, to: string): void {
  const fromWords = from.toLowerCase().split(' ');
  const toWords = to.split(' ');

  // Build an index of only word tokens (with their position in the full array)
  const wordPositions: number[] = [];
  for (let i = 0; i < tokens.length; i++) {
    if (tokens[i].isWord) wordPositions.push(i);
  }

  for (let wi = 0; wi <= wordPositions.length - fromWords.length; wi++) {
    // Check whether fromWords.length consecutive word tokens match from (case-insensitive)
    let match = true;
    for (let fi = 0; fi < fromWords.length; fi++) {
      const tok = tokens[wordPositions[wi + fi]];
      if (tok.locked || tok.text.toLowerCase() !== fromWords[fi]) {
        match = false;
        break;
      }
    }
    if (!match) continue;

    // Replace the matched word tokens with toWords, mark all as locked.
    // We replace in-place; if counts differ we remove/insert tokens.
    const matchedIndices = fromWords.map((_, fi) => wordPositions[wi + fi]);

    // Build new token objects for replacement words
    const replacements: Token[] = toWords.map(w => ({
      text: w,
      locked: true,
      isWord: true,
    }));

    // Replace tokens[matchedIndices[0]] .. tokens[matchedIndices[last]] span
    // with replacements, preserving any gap tokens in between.
    const spanStart = matchedIndices[0];
    const spanEnd = matchedIndices[matchedIndices.length - 1];

    // Collect gap tokens that sit between the matched word tokens
    const gapsBetween: Token[][] = [];
    for (let fi = 0; fi < fromWords.length - 1; fi++) {
      const gapStart = matchedIndices[fi] + 1;
      const gapEnd = matchedIndices[fi + 1];
      gapsBetween.push(tokens.slice(gapStart, gapEnd));
    }

    // Build the replacement span: interleave replacement words with the
    // original gaps (to preserve spacing)
    const newSpan: Token[] = [];
    for (let ri = 0; ri < replacements.length; ri++) {
      newSpan.push(replacements[ri]);
      if (ri < gapsBetween.length) {
        newSpan.push(...gapsBetween[ri]);
      }
    }

    tokens.splice(spanStart, spanEnd - spanStart + 1, ...newSpan);

    // Rebuild wordPositions from scratch since splice changed indices
    wordPositions.length = 0;
    for (let i = 0; i < tokens.length; i++) {
      if (tokens[i].isWord) wordPositions.push(i);
    }
  }
}

/**
 * Apply pronoun/grammar swaps to captured wildcard words.
 * Exact C swap order — order matters to avoid double-swapping.
 * Mirrors _sp_fix.
 */
export function applyGrammarFix(text: string): string {
  const tokens = tokenize(text);
  applySwap(tokens, 'that you', 'that I');
  applySwap(tokens, 'my', 'your');
  applySwap(tokens, 'you', 'me');
  applySwap(tokens, 'your', 'my');
  applySwap(tokens, 'me', 'you');
  applySwap(tokens, 'mine', 'yours');
  applySwap(tokens, 'am', 'are');
  applySwap(tokens, 'yours', 'mine');
  applySwap(tokens, 'yourself', 'myself');
  applySwap(tokens, 'myself', 'yourself');
  applySwap(tokens, 'are', 'am');
  applySwap(tokens, 'i', 'you');
  return detokenize(tokens);
}

/**
 * Post-substitution grammar fixes applied to the full response string.
 * These correct grammatical artifacts that arise after % substitution
 * (e.g. "i are" → "I am", "me have" → "I have").
 * Mirrors the second batch of _sp_gswap calls in C's _sp_usetempl.
 */
export function applyPostGrammarFix(text: string): string {
  const tokens = tokenize(text);
  applySwap(tokens, 'i are', 'I am');
  applySwap(tokens, 'me have', 'I have');
  applySwap(tokens, "me don't", "I don't");
  applySwap(tokens, 'you am', 'you are');
  applySwap(tokens, 'me am', 'I am');
  applySwap(tokens, 'i', 'I');
  applySwap(tokens, 'did not be', "weren't");
  return detokenize(tokens);
}
