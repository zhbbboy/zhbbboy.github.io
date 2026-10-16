// @ts-check
import { existsSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { slug } from 'github-slugger';

const postsRoot = fileURLToPath(new URL('../src/content/posts/', import.meta.url));

/** @param {string} filePath */
function isPost(filePath) {
  const relative = path.relative(postsRoot, filePath);
  return relative !== '' &&
    relative !== '..' &&
    !relative.startsWith('..' + path.sep) &&
    !path.isAbsolute(relative);
}

/**
 * Keep source links usable in Obsidian; convert relative Markdown links only
 * when rendering the blog. Slugs match the content collection's default loader.
 *
 * @returns {import('unified').Transformer<import('mdast').Root>}
 */
export default function remarkNoteLinks() {
  return (tree, file) => {
    if (!file.path || !isPost(file.path)) return;

    /** @param {string} url */
    function postUrl(url) {
      // External URLs, site routes, and same-page anchors already have meaning.
      if (/^(?:[a-z][a-z\d+.-]*:|\/|#|\?)/i.test(url)) return url;
      const parts = /^([^?#]+)([?#].*)?$/.exec(url);
      if (!parts) return url;

      let relativeTarget;
      try {
        relativeTarget = decodeURIComponent(parts[1] ?? '');
      } catch {
        return url;
      }
      if (!/\.mdx?$/i.test(relativeTarget)) return url;

      const target = path.resolve(path.dirname(file.path), relativeTarget);
      if (!isPost(target) || !existsSync(target)) return url;

      const relative = path.relative(postsRoot, target)
        .replace(/\.mdx?$/i, '');
      const id = relative.split(path.sep)
        .map(segment => slug(segment))
        .join('/')
        .replace(/\/index$/, '');
      return '/posts/' + id + (parts[2] ?? '');
    }

    /** @param {import('mdast').Nodes} node */
    function visit(node) {
      if (node.type === 'link' || node.type === 'definition') {
        node.url = postUrl(node.url);
      }
      if ('children' in node) {
        for (const child of node.children) visit(child);
      }
    }

    visit(tree);
  };
}
