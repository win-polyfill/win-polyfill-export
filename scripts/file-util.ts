import * as fs from "fs/promises";
import * as path from "path";

export async function getFiles(dir: string, recursive: boolean = false) {
  const subdirs = await fs.readdir(dir);
  let files:string[] = []
  await Promise.all(subdirs.map(async (name) => {
    const res = path.resolve(dir, name);
    if ((await fs.stat(res)).isDirectory()) {
      if (!recursive)
          return
      for (let f of await getFiles(res, recursive)) {
        files.push(f)
      }
    } else {
      files.push(res);
    }
  }));
  return files;
}

export async function readJson(p: string) {
  const content = await fs.readFile(p, { encoding: 'utf-8' });
  return JSON.parse(content);
}
