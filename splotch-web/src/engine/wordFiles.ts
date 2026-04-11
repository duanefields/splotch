/**
 * Expand @filename.w references in response strings.
 * The C code reads exactly two chars after the '.' giving extension ".w",
 * so "@animal.ws" → reads "animal.w" and leaves the trailing "s" as literal
 * text (producing the plural form). We replicate this with /@([\w]+)\.w/g.
 */

export function expandWordFileRefs(
  text: string,
  wordFiles: Map<string, string[]>,
  rng: () => number,
  maxDepth = 10,
): string {
  if (maxDepth === 0) return text;

  return text.replace(/@([\w]+)\.w/g, (_match, name: string) => {
    const words = wordFiles.get(name) ?? wordFiles.get(name.replace(/s$/, '')) ?? [];
    if (words.length === 0) return '';
    const picked = words[Math.floor(rng() * words.length)];
    // Recurse in case the picked line itself contains @refs
    return expandWordFileRefs(picked, wordFiles, rng, maxDepth - 1);
  });
}
