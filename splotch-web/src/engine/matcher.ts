import type { DictEntry } from './types';

export interface MatchResult {
  matchIndex: number;    // -1 if no match found
  capturedWords: string; // text captured by a % wildcard
}

/**
 * Find the first word-boundary occurrence of `search` in `text` (case-insensitive).
 * Mirrors C's _sp_phrasefind which uses isalnum for boundary detection.
 * Returns the index of the match, or null if not found.
 */
export function phraseFind(text: string, search: string): number | null {
  if (!search) return null;
  const escaped = search.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const re = new RegExp(`(?<![a-zA-Z0-9])${escaped}(?![a-zA-Z0-9])`, 'i');
  const m = re.exec(text);
  return m ? m.index : null;
}

/**
 * Attempt a wildcard match: find `pre` in the question, then optionally `post` after it.
 * Returns the captured text between pre and post (or to end if no post), or null.
 * Mirrors the % wildcard logic in C's _sp_trytempl.
 */
function wildcardFind(question: string, pre: string, post: string): string | null {
  const preIdx = phraseFind(question, pre);
  if (preIdx === null) return null;

  // Start of captured text = end of pre + 1 space
  const afterPre = preIdx + pre.length;
  // Skip the space that separates the pre-phrase from the wildcard content
  const captureStart = afterPre + (question[afterPre] === ' ' ? 1 : 0);
  const tail = question.slice(captureStart);

  if (!tail.trim()) return null;

  if (!post) {
    // Trailing wildcard: "phrase %" — capture everything after pre
    return tail;
  }

  // Sandwiched wildcard: "pre % post" — capture between pre and post
  const postIdx = phraseFind(tail, post);
  if (postIdx === null) return null;
  // Trim trailing space before post
  const captured = tail.slice(0, postIdx).replace(/\s+$/, '');
  return captured || null;
}

/**
 * Try all templates and return the best (highest priority) match.
 * Mirrors C's _sp_trytempl.
 */
export function findMatch(
  question: string,
  entries: DictEntry[],
  botName: string,
): MatchResult {
  let winnerIndex = -1;
  let winScore = 0;
  let winWords = '';

  for (const entry of entries) {
    let orMatched = false;
    let mandatoryFailed = false;
    let capturedWords = '';

    for (const clause of entry.clauses) {
      if (mandatoryFailed) break;

      switch (clause.type) {
        case 'NOT':
          if (phraseFind(question, clause.phrase) !== null) {
            mandatoryFailed = true;
          }
          break;

        case 'AND':
          if (phraseFind(question, clause.phrase) === null) {
            mandatoryFailed = true;
          }
          break;

        case 'NAME':
          if (phraseFind(question, botName) === null) {
            mandatoryFailed = true;
          }
          break;

        case 'OR':
          if (phraseFind(question, clause.phrase) !== null) {
            orMatched = true;
          }
          break;

        case 'WILDCARD': {
          const captured = wildcardFind(
            question,
            clause.wildcardPre ?? '',
            clause.wildcardPost ?? '',
          );
          if (captured !== null) {
            orMatched = true;
            capturedWords = captured;
          }
          break;
        }
      }
    }

    if (mandatoryFailed || !orMatched) continue;

    if (entry.priority > winScore) {
      winnerIndex = entry.id;
      winScore = entry.priority;
      winWords = capturedWords;
    }

    // Priority 9 = immediate return (highest possible, no point searching further)
    if (entry.priority === 9) {
      return { matchIndex: winnerIndex, capturedWords: winWords };
    }
  }

  return { matchIndex: winnerIndex, capturedWords: winWords };
}
