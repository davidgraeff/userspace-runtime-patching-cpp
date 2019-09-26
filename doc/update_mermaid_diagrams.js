// MIT license - David Gräff
// This script is calling the mermaid js package which renders pictures
// as svg or png via a sandbox headless browser.
const commander = require('commander')
const chalk = require('chalk')
const fs = require('fs')
const path = require('path')
const puppeteer = require('puppeteer')

const error = message => {
    console.log(chalk.red(`\n${message}\n`))
    process.exit(1)
}

const checkConfigFile = file => {
    if (!fs.existsSync(file)) {
        error(`Configuration file "${file}" doesn't exist`)
    }
}

commander
    .option('-t, --theme [theme]', 'Theme of the chart, could be default, forest, dark or neutral. Optional. Default: default', /^default|forest|dark|neutral$/, 'default')
    .option('-w, --width [width]', 'Width of the page. Optional. Default: 800', /^\d+$/, '800')
    .option('-H, --height [height]', 'Height of the page. Optional. Default: 600', /^\d+$/, '600')
    .option('-b, --backgroundColor [backgroundColor]', 'Background color. Example: transparent, red, \'#F0F0F0\'. Optional. Default: white')
    .option('-c, --configFile [configFile]', 'JSON configuration file for mermaid. Optional')
    .option('-C, --cssFile [cssFile]', 'CSS file for the page. Optional')
    .option('-p --puppeteerConfigFile [puppeteerConfigFile]', 'JSON configuration file for puppeteer. Optional')
    .parse(process.argv)

let { theme, width, height, backgroundColor, configFile, cssFile, puppeteerConfigFile } = commander

// check config files
let mermaidConfig = { theme }
if (configFile) {
    checkConfigFile(configFile)
    mermaidConfig = Object.assign(mermaidConfig, JSON.parse(fs.readFileSync(configFile, 'utf-8')))
}
let puppeteerConfig = {}
if (puppeteerConfigFile) {
    checkConfigFile(puppeteerConfigFile)
    puppeteerConfig = JSON.parse(fs.readFileSync(puppeteerConfigFile, 'utf-8'))
}

// check cssFile
let myCSS
if (cssFile) {
    if (!fs.existsSync(cssFile)) {
        error(`CSS file "${cssFile}" doesn't exist`)
    }
    myCSS = fs.readFileSync(cssFile, 'utf-8')
} else {
    myCSS = ".node rect, rect.actor, .node polygon, polygon.labelBox, line.loopLine { fill: #ff8a65; stroke: #ff8a65; } text.actor, text.labelText>tspan {fill: white;font-size: large;} .label { color: white; } .edgeLabel { color: black; background-color: white; font-size: small } .label foreignObject { overflow: visible; }";
}

mermaidConfig = Object.assign(mermaidConfig, { themeCSS: myCSS });

// normalize args
width = parseInt(width)
height = parseInt(height)
backgroundColor = backgroundColor || 'transparent';

async function processDef(browser, definition, output, config = {}) {
    const page = await browser.newPage()
    page.setViewport({ width, height })
    await page.goto(`file://${path.join(__dirname, 'node_modules/mermaid.cli/index.html')}`)
    if (backgroundColor)
        await page.evaluate(`document.body.style.background = '${backgroundColor}'`)

    page.on('console', consoleObj => console.log(consoleObj.text()));
    await page.$eval('#container', (container, definition, config) => {
        container.innerHTML = definition
        window.mermaid.initialize(config)
        window.mermaid.init(undefined, container)
    }, definition, config)

    if (output.endsWith('svg')) {
        const svg = await page.$eval('#container', container => container.innerHTML)
        fs.writeFileSync(output, svg)
    } else if (output.endsWith('png')) {
        const clip = await page.$eval('svg', svg => {
            const react = svg.getBoundingClientRect()
            return { x: react.left, y: react.top, width: react.width, height: react.height }
        })
        await page.screenshot({ path: output, clip, omitBackground: backgroundColor === 'transparent' })
    } else { // pdf
        await page.pdf({ path: output, printBackground: backgroundColor !== 'transparent' })
    }
    await page.close();
}

function* walk(dir) {
    var list = fs.readdirSync(dir);
    for (let file of list) {
        file = dir + '/' + file;
        var stat = fs.statSync(file);
        if (stat && stat.isDirectory()) {
            yield* walk(file);
        } else {
            yield file;
        }
    };
}

let regex = /\{\{<\ mermaid([^>]*)>\}\}(.+?)(?=\{\{)/gs
let regexContext = /context="([^"]*)"/
let regexOptions = /options="([^"]*)"/

async function* getDefinitions() {
    for (let file of walk("./diagrams")) {
        if (!file.endsWith(".mermaid")) continue;
        const content = fs.readFileSync(file, 'utf-8');
        yield { file: path.basename(file), options:{}, context:path.basename(file,".mermaid"), definition: content }
    }
}

async function main() {
    const browser = await puppeteer.launch(puppeteerConfig);

    for await (let d of getDefinitions()) {
        let config = Object.assign(mermaidConfig, d.options);
        config.themeCSS = myCSS;

        let out = __dirname + "/" + d.context + ".svg";
        console.log("Render file", out, d.context);
        await processDef(browser, d.definition, out, config);
        out = __dirname + "/" + d.context + ".png";
        console.log("Render file", out, d.context);
        await processDef(browser, d.definition, out, config);
    }
    browser.close();
}

main();
