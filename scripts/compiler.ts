import { Deferred } from "@esutils/deferred";

declare global {
  interface String {
    last(): string;
    splice(idx: number, rem: number, s: string): string;
    getNextString(): string;
    isAlpha(i: number): boolean;
  }
}

// Return the last character of the string
String.prototype.last = function () {
  return this.slice(-1);
};

// Remove and add some text in the same time
String.prototype.splice = function (idx, rem, s) {
  return this.slice(0, idx) + s + this.slice(idx + rem);
};

// Get the next "..." string
String.prototype.getNextString = function () {
  var str = this.match(/^"([A-Za-z0-9\-_\. \/\\]+)"/);
  return !str || !str[1] ? "" : str[1];
};

// Test if a character is alpha numeric or _
var StringArray =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";

String.prototype.isAlpha = function (i) {
  return StringArray.indexOf(this[i]) != -1;
};

interface Constants {
  [key: string]: string;
}

interface Macros {
  [key: string]: string;
}

export interface CompilerOptions {
  newLine?: string;
  commentEscape?: boolean;
  includeSpaces?: number;
  emptyLinesLimit?: number;
  basePath?: string;
  stopOnError?: boolean;
  enumInHex?: boolean;
  filename?: string;
  macros?: Macros;
  constants?: Constants;
}

interface Define {
  name: string;
  value: string;
  count?: number;
}
interface Macro {
  name: string;
  count: number;
  content: string[];
  pos: number[];
}

interface Directive {
  [key: string]: Function;
}

export class Compiler {
  options: CompilerOptions;
  defines: { [key: string]: Define | Macro };
  stack: { [key: string]: (Define | Macro)[] };
  includeOnce: { [key: string]: boolean };
  Directives: Directive = {};

  currentLine: number = 0;
  linesCount: number = 0;
  lines: string[] = [];
  currentFile: string = "";
  emptyLines: number = 0;
  result: string = "";
  running: boolean = false;

  constructor(opt: CompilerOptions) {
    let defaultOptions: CompilerOptions = {
      newLine: "\n",
      commentEscape: true,
      includeSpaces: 0,
      emptyLinesLimit: 0,
      basePath: "./",
      stopOnError: true,
      enumInHex: true,
    };
    this.options = {
      ...defaultOptions,
      ...opt,
    };
    this.defines = {};
    this.stack = {};
    this.includeOnce = {};

    const date = new Date();
    this._compConst("TIME", date.toLocaleTimeString());
    this._compConst("DATE", date.toLocaleDateString());
    this._compConst("LINE", "0");
    this._compConst("FILE", "main");

    // User defined constants
    var c = opt.constants || {};
    for (var i in c) this.createConstant(i, c[i].toString(), false);

    // User defined macros
    var m = opt.macros || {};
    for (var i in m) this.createMacro(i, m[i]);

    this.createDirective("define", (text: string) => {
      let i: number = 0;
      while (text.isAlpha(i)) i++;

      const name: string = text.substr(0, i);
      const isMacro: boolean = text[i] == "(";

      text = text.substr(name.length).trimLeft();

      let str: string = text.trimRight();
      text = "";

      while (str.last() == "\\") {
        text += str.substr(0, str.length - 1) + this.options.newLine;
        str = this.nextLine().trimRight();
      }

      text += str;

      let posBegin: number;
      let posEnd: number;

      while ((posBegin = text.indexOf("/*")) != -1) {
        posEnd = text.indexOf("*/", 1 + posBegin);
        if (posEnd == -1) posEnd = text.length;

        text = text.substring(0, posBegin) + " " + text.substring(2 + posEnd);
      }

      if ((posBegin = text.indexOf("//")) != -1)
        text = text.substring(0, posBegin) + " ";

      text.trimRight();

      if (isMacro) this.createMacro(name, text);
      else this.createConstant(name, text);
    });

    this.createDirective("undef", (text: string): void => {
      let i: number = 0;
      while (text.isAlpha(i)) i++;

      const name: string = text.substr(0, i);

      delete this.defines[name];
    });

    this.createDirective("if", (expr: string): void => {
      let r = false;

      expr = expr.replace(/defined\s*\(\s*([\s\S]+?)\s*\)/g, (match, p1) => {
        return this.defines[p1] === undefined ? "false" : "true";
      });

      expr = this.addDefines(expr);

      try {
        r = eval(expr);
      } catch (e) {
        throw new Error("error when evaluating #if expression");
      }

      if (!r) this.conditionNext();
    });

    this.createDirective("ifdef", (text: string): void => {
      const name: string = text.split(" ")[0];

      if (this.defines[name] === undefined) this.conditionNext();
    });

    this.createDirective("ifndef", (text: string): void => {
      const name: string = text.split(" ")[0];

      if (this.defines[name] !== undefined) this.conditionNext();
    });

    this.createDirective("elif", (expr: string, called: boolean): void => {
      if (!called) return this.conditionEnd();

      this.Directives.if.call(this, expr);
    });

    this.createDirective("else", (expr: string, called: boolean): void => {
      if (!called) return this.conditionEnd();
    });

    this.createDirective("endif", (expr: string, called: boolean): void => {
      // Do nothing because this directive is already evaluated by 'this.conditionNext'
    });

    this.createDirective("pragma", (text: string): void => {
      text = text.trim();

      if (text == "once") this.includeOnce[this.currentFile] = true;
      else if (text.startsWith("push_macro")) {
        const match = text.match(/push_macro\("([^"]+)"\)/);

        if (match === null || match[1].length == 0)
          throw new Error(`wrong pragma format`);
        else this.pushMacro(match[1]);
      } else if (text.startsWith("pop_macro")) {
        const match = text.match(/pop_macro\("([^"]+)"\)/);

        if (match === null || match[1].length == 0)
          throw new Error(`wrong pragma format`);
        else this.popMacro(match[1]);
      } else {
        // throw new Error(`unknown pragma "${text}"`);
        // all other pragma are ignored
      }
    });

    this.createDirective("error", (text: string): void => {
      throw new Error(text.trim());
    });
  }

  // Go to the end of a multilines comment
  commentEnd() {
    this.currentLine--;
    var line, i;

    // Find the end of the comment
    while (this.currentLine < this.linesCount) {
      line = this.nextLine();

      if (line.indexOf("*/") != -1) break;
    }
  }

  // Go to the end of the condtion (#endif)
  conditionEnd() {
    this.conditionNext(true);
  }

  // Go to the next #elif, #else or #endif
  conditionNext(end: boolean = false) {
    // #if directives to start a condition
    var ifCmd = ["if", "ifdef", "ifndef"];

    // #else directives
    var elseCmd = ["elif", "else"];

    // Local variables
    var line,
      s,
      n = 1;

    // Count unexploited conditions
    while (this.currentLine < this.linesCount) {
      line = this.nextLine().trimLeft();
      if (line[0] != "#") continue;

      s = line.substr(1).trimLeft().split(" ")[0];

      if (ifCmd.indexOf(s) != -1) n++;
      else if (!end && n == 1 && elseCmd.indexOf(s) != -1)
        return this.callCondition(line);
      else if (s == "endif") {
        n--;
        if (n == 0) return;
      }
    }
  }

  // Call a #else or #elif condition
  callCondition(text: string) {
    // Get the directive name
    var split = text.substr(1).trimLeft().split(" "),
      name = split[0];

    // Get the remaining text (without the # directive)
    split.shift();
    text = split.join(" ").trimLeft();

    // Call the corresponding directive
    this.Directives[name].call(this, text, true);
  }

  createDirective(name: string, fn: Function): void {
    this.Directives[name] = fn;
  }

  // Return the next line
  nextLine() {
    this._compConst("LINE", `${this.currentLine + 1}`);
    return this.lines[this.currentLine++];
  }

  // Parse the next lines (doing it synchronously until an asynchronous command)
  next() {
    var running = true;
    while (this.running && running && this.currentLine < this.linesCount)
      running = this.parseNext() !== false;

    if (this.running && running && this.currentLine >= this.linesCount)
      return false;
  }

  compile(code: string, filename?: string): string {
    if (filename) this.options.filename = filename;

    // Set the processor as running
    this.running = true;

    // Get an array of all lines
    this.lines = code.split(/\r?\n/);
    this.linesCount = this.lines.length;

    // Parse the first line
    this.next();
    return this.result;
  }

  // Append a line to the result
  addLine(line: string) {
    this.result += line + this.options.newLine;
    this.emptyLines = 0;
  }

  parseNext() {
    // No more line to parse: stop this function
    if (this.currentLine >= this.linesCount) return;

    // Get the next line text
    var line = this.nextLine(),
      text = line.trimLeft();

    // If the line is empty: apply empty lines limit option
    if (text.length == 0) {
      if (
        this.options.emptyLinesLimit &&
        this.emptyLines >= this.options.emptyLinesLimit
      )
        return;

      this.emptyLines++;
      return this.addLine(line);
    }

    // If the line starts with a # comment: delete it
    if (this.options.commentEscape && text.startsWith("//#")) return;

    if (this.options.commentEscape && text.startsWith("/*#"))
      return this.commentEnd();

    // If the line doesn't start with #
    if (text[0] != "#") return this.addLine(this.addDefines(line));

    // Get the # directive and the remaing text
    var i = text.indexOf(" "),
      name;

    if (i != -1) {
      name = text.substr(1, i - 1);
      text = text.substr(i + 1);
    } else {
      name = text.substr(1);
    }

    // Get the # directive
    var cmd = this.Directives[name.trimLeft()];

    // If the command exists: call the corresponding function
    if (cmd) return cmd.call(this, text);

    // Else: remove the line if 'commentEscape' is enabled
    if (!this.options.commentEscape) this.addLine(this.addDefines(line));
  }

  // Set a compiler constant
  private _compConst(name: string, value: string): void {
    this.createConstant("__" + name + "__", value, false);
  }

  // Create a constant
  createConstant(
    name: string,
    value: string,
    addDefines: boolean = true
  ): void {
    if (addDefines) value = this.addDefines(value);
    this.defines[name] = { name, value };
  }

  // Add defined objects to a line
  private addDefines(
    line: string,
    withConst: boolean = true,
    withMacros: boolean = true
  ): string {
    // Check if the constant is present in the line
    for (let [i, d] of Object.entries(this.defines)) {
      if (d.count !== undefined && withMacros === false) continue;
      if (d.count == undefined && withConst === false) continue;

      var i2 = i.length;
      var i1 = -1;

      // It can have the same constant more than one time
      for (;;) {
        // Get the position of the constant (-1 if not present)
        i1 = line.indexOf(i, i1 + 1);
        if (i1 == -1) break;

        // Check that the constant isn't in a middle of a word and add the constant if not
        if (line.isAlpha(i1 - 1) || line.isAlpha(i1 + i2)) continue;

        // Add the macro or the constant
        // Local variables
        var r;
        if (d.count !== undefined) r = this.addMacro(line, i1, d as Macro);
        else r = this.addConstant(line, i1, d as Define);

        line = r.line;
        i1 = r.index;
      }
    }

    return line;
  }

  // Add a constant in a line
  addConstant(line: string, i: number, constant: Define) {
    line = line.splice(i, constant.name.length, constant.value);
    i += constant.value.length;

    return { line: line, index: i };
  }

  // Read a line and transform macro by adding their value
  addMacro(
    line: string,
    i: number,
    macro: Macro
  ): { line: string; index: number } {
    // Local variables
    var m = 0;
    let args = [] as string[];
    var e = i + macro.name.length;

    // Get arguments between parenthesis (by counting parenthesis)
    for (;;) {
      m = 0;
      args = [];
      e = i + macro.name.length;
      var s = e;

      for (var v, l = line.length; e < l; e++) {
        v = line[e];

        if (v == "(") {
          m++;
          if (m == 1) s = e + 1;
        } else if (v == "," && m == 1) {
          args.push(line.slice(s, e));
          s = e + 1;
        } else if (v == ")") {
          m--;
          if (m < 0)
            throw new Error(
              `there is no openning parenthesis for macro ${macro.name}`
            );
          if (m === 0) break;
        }
      }
      if (m === 0) {
        break;
      }
      line = line + this.nextLine().trimStart();
    }

    // If the closing parenthesis is missing
    if (m != 0)
      throw new Error(
        `the closing parenthesis is missing for macro ${macro.name}`
      );

    // Add the last argument.
    var lastarg = line.slice(s, e);
    if (lastarg.trim()) args.push(lastarg);

    // Check if there is the right number of arguments
    if (args.length > macro.count)
      throw new Error(`too many arguments for macro ${macro.name}`);

    if (args.length < macro.count)
      throw new Error(`too few arguments for macro ${macro.name}`);

    // Execute 'addDefines' on each argument
    for (var j = 0; j < macro.count; j++) args[j] = this.addDefines(args[j]);

    // Replace macro variables with the given arguments
    var str = macro.content[0];

    for (let si = 0, l = macro.pos.length; si < l; si++)
      str += args[macro.pos[si]] + macro.content[si + 1];

    // Add the result into the line
    line = line.splice(i, e - i + 1, str);
    i += str.length;

    return { line: line, index: i };
  }

  // Create a macro (text must have the macro arguments, like this: '(a,b) a+b')
  createMacro(name: string, text: string): void {
    // First, get macro arguments
    var args = [];

    var end = text.indexOf(")"),
      i1 = 1,
      i2 = 0;

    // If there is no closing parenthesis
    if (end == -1)
      throw new Error(
        `no closing parenthesis in the #define of marcro ${name}`
      );

    // Get arguments
    while ((i2 = text.indexOf(",", i2 + 1)) != -1 && i2 < end) {
      args.push(text.substring(i1, i2).trim());
      i1 = i2 + 1;
    }

    if (end > i1)
      // If there is at least one argument.
      args.push(text.substring(i1, end));

    // Remove arguments in the text
    text = text.substr(end + 1).trimLeft();

    // Execute defined macros
    text = this.addDefines(text, false, true);

    // Secondly, makes breaks and store variables positions
    var breaks = [];

    for (var i = 0, l = args.length, p; i < l; i++) {
      i1 = -1;
      p = args[i];
      i2 = p.length;

      for (;;) {
        i1 = text.indexOf(p, i1 + 1);
        if (i1 == -1) break;

        if (text.isAlpha(i1 - 1) || text.isAlpha(i1 + i2)) continue;

        breaks.push([i1, i, i2]);
      }
    }

    // Sort variables in order of their positions in the macro text
    breaks.sort(function (a, b) {
      return a[0] - b[0];
    });

    // Thirdly, cut the text into parts without variable and add defined constants
    var offset = 0;
    let content = [] as string[];
    let pos = [] as number[];

    for (let i = 0; i < breaks.length; i++) {
      content.push(
        this.addDefines(text.slice(offset, breaks[i][0]), true, false)
      );
      offset = breaks[i][0] + breaks[i][2];
      pos.push(breaks[i][1]);
    }

    content.push(this.addDefines(text.slice(offset)));

    // Fourthly, store the macro
    let macro: Macro = {
      content: content,
      count: args.length,
      pos: pos,
      name: name,
    };
    this.defines[name] = macro;
  }

  // Save the current value of a macro on top of the stack
  pushMacro(name: string) {
    if (this.defines[name] === undefined)
      throw new Error(`macro ${name} is not defined, cannot push it`);

    if (this.stack[name] === undefined) this.stack[name] = [];

    this.stack[name].push(this.defines[name]);
  }

  // Set current value of the specified macro to previously saved value
  popMacro(name: string) {
    let macro = this.stack[name]?.pop();
    if (macro === undefined)
      throw new Error(`stack for macro ${name} is empty, cannot pop from it`);

    this.defines[name] = macro;
  }
}

const constants = {
  JET_VERSION: "0x0B00",
  REVISED_CONFIG_APIS: "1",
};
const macros = {
  _When_: "(c,a)",
};

export async function preprocessor(
  code: string,
  options: CompilerOptions
): Promise<string | null> {
  let deferred = new Deferred<string>();
  try {
  } catch (error) {
    deferred.reject(error);
  }
  return deferred.promise;
}

export async function handle_preprocess(
  str: string,
  key?: string
): Promise<string> {
  try {
    let options = {
      constants: constants,
      macros: macros,
    };
    let compiler = new Compiler(options);
    return compiler.compile(str);
  } catch (error) {
    if (error instanceof Error) {
      console.log(
        `Handle preprocessor for ${key} code:"${str}" ${error} ${error.stack} failed`
      );
    } else {
      console.log(
        `Handle preprocessor for ${key} code:"${str}" ${error} failed`
      );
    }
    process.exit(-1);
  }
}

async function test_func() {
  let test = `(
  _In_ HKEY hkey,
  _In_opt_ LPCSTR lpSubKey,
  _In_opt_ LPCSTR lpValue,
  _In_ DWORD dwFlags,
  _Out_opt_ LPDWORD pdwType,
    _When_((dwFlags & 0x7F) == RRF_RT_REG_MULTI_SZ ||
    *pdwType == REG_MULTI_SZ, _Post_ _NullNull_terminated_)
  _Out_writes_bytes_to_opt_(*pcbData,*pcbData) PVOID pvData,
  _Inout_opt_ LPDWORD pcbData
  )`;
  let code = await handle_preprocess(test);
  console.log(code);
}

// test_func();
