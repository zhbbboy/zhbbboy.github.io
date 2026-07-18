import { getCollection, type CollectionEntry } from 'astro:content';
import readingTime from 'reading-time';

export type Post = CollectionEntry<'posts'>;

export interface PostFolder {
  /** Folder name as it appears under src/content/posts. */
  name: string;
  /** Relative folder path, useful as a stable key when rendering. */
  path: string;
  folders: PostFolder[];
  posts: Post[];
  postCount: number;
}

/** Folder segments from the original Markdown path, without the file name. */
export function getPostFolderSegments(post: Post): string[] {
  const sourcePath = post.filePath?.replaceAll('\\', '/');
  const contentRoot = 'src/content/posts/';
  const sourceRelativePath = sourcePath?.startsWith(contentRoot)
    ? sourcePath.slice(contentRoot.length)
    : post.id;

  return sourceRelativePath.split('/').slice(0, -1);
}

function getPostSourceName(post: Post): string {
  const sourcePath = post.filePath?.replaceAll('\\', '/');
  const sourceFileName = sourcePath?.split('/').at(-1);

  return sourceFileName?.replace(/\.(?:md|mdx)$/i, '') ?? post.data.title;
}

export async function getPublishedPosts(): Promise<Post[]> {
  const all = await getCollection('posts', ({ data }) => !data.draft || import.meta.env.DEV);
  return all.sort((a, b) => b.data.date.valueOf() - a.data.date.valueOf());
}

export function estimateReadingMinutes(body: string): number {
  return Math.max(1, Math.round(readingTime(body).minutes));
}

export function groupByYear(posts: Post[]): Map<number, Post[]> {
  const map = new Map<number, Post[]>();
  for (const post of posts) {
    const year = post.data.date.getFullYear();
    const bucket = map.get(year);
    if (bucket) bucket.push(post);
    else map.set(year, [post]);
  }
  return map;
}

/**
 * Builds a navigable folder tree from the content collection IDs.
 *
 * The source file path is used for labels because Astro normalises punctuation
 * in content IDs. This keeps folder names in sync with the author's disk
 * structure (for example, `1.cpp基础` remains `1.cpp基础`).
 */
export function buildPostFolderTree(posts: Post[]): PostFolder[] {
  interface MutableFolder {
    name: string;
    path: string;
    folders: Map<string, MutableFolder>;
    posts: Post[];
  }

  const root: MutableFolder = {
    name: '',
    path: '',
    folders: new Map(),
    posts: [],
  };

  for (const post of posts) {
    const folderParts = getPostFolderSegments(post);
    let current = root;

    for (const name of folderParts) {
      const path = current.path ? `${current.path}/${name}` : name;
      let folder = current.folders.get(name);

      if (!folder) {
        folder = { name, path, folders: new Map(), posts: [] };
        current.folders.set(name, folder);
      }

      current = folder;
    }

    current.posts.push(post);
  }

  const collator = new Intl.Collator('zh-CN', { numeric: true, sensitivity: 'base' });
  const sortByFileName = (a: Post, b: Post) =>
    collator.compare(getPostSourceName(a), getPostSourceName(b)) ||
    a.data.date.valueOf() - b.data.date.valueOf();

  const toFolder = (folder: MutableFolder): PostFolder => {
    const folders = [...folder.folders.values()]
      .sort((a, b) => collator.compare(a.name, b.name))
      .map(toFolder);
    const postsInFolder = [...folder.posts].sort(sortByFileName);

    return {
      name: folder.name,
      path: folder.path,
      folders,
      posts: postsInFolder,
      postCount: postsInFolder.length + folders.reduce((total, item) => total + item.postCount, 0),
    };
  };

  return [...root.folders.values()]
    .sort((a, b) => collator.compare(a.name, b.name))
    .map(toFolder);
}

export function collectTags(posts: Post[]): Map<string, number> {
  const map = new Map<string, number>();
  for (const post of posts) {
    for (const tag of post.data.tags) {
      map.set(tag, (map.get(tag) ?? 0) + 1);
    }
  }
  return map;
}

export function formatDate(date: Date): string {
  const y = date.getFullYear();
  const m = String(date.getMonth() + 1).padStart(2, '0');
  const d = String(date.getDate()).padStart(2, '0');
  return `${y}-${m}-${d}`;
}
