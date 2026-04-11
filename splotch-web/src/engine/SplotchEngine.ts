import type { SplotchData, DictEntry } from './types';
import { expandSynonyms } from './expander';
import { findMatch } from './matcher';
import { buildResponse } from './responder';

export class SplotchEngine {
  private entries: DictEntry[];
  private defaultIndex: number;
  private data: SplotchData;
  private botName: string;
  private history: number[];
  private rng: () => number;

  constructor(data: SplotchData, botName = 'splotch', rng = Math.random.bind(Math)) {
    this.data = data;
    this.entries = data.entries;
    this.defaultIndex = data.defaultIndex;
    this.botName = botName.toLowerCase();
    this.rng = rng;
    this.history = new Array(10).fill(-1);

    // Initialize each entry's nextIndex to a random starting position
    for (const entry of this.entries) {
      entry.nextIndex = Math.floor(rng() * entry.responses.length);
    }
  }

  /**
   * Process a user message and return Splotch's response.
   * Mirrors the flow: _sp_expand → _sp_trytempl → _sp_usetempl.
   */
  ask(userName: string, userText: string): string {
    // Prepend "Name: " as the original chat.c does (influences NAME clause matching)
    const question = `${userName.toLowerCase()}: ${userText}`;

    // Expand synonyms and lowercase
    const expanded = expandSynonyms(question, this.data.synonyms);

    // Find best matching template
    const { matchIndex, capturedWords } = findMatch(
      expanded,
      this.entries,
      this.botName,
    );

    let templateIndex: number;
    if (matchIndex >= 0) {
      const matched = this.entries[matchIndex];
      // Track priority-6 matches in history (mirrors C history queue)
      if (matched.priority === 6) {
        this.history.shift();
        this.history.push(matchIndex);
      }
      templateIndex = matchIndex;
    } else {
      // No match — use the fallback (default) response
      templateIndex = this.defaultIndex;
    }

    const entry = this.entries[templateIndex];
    return buildResponse(entry, capturedWords, this.botName, this.data.wordFiles, this.rng);
  }
}
