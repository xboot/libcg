module.exports = {
  title: 'libcg 文档',
  url: 'https://libcg.xboot.org',
  baseUrl: '/',
  onBrokenLinks: 'warn',
  markdown: {
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },
  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          path: 'docs',
          routeBasePath: '/',
          sidebarPath: require.resolve('./sidebars.js'),
        },
        blog: false,
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],
  plugins: [
    [
      require.resolve('@easyops-cn/docusaurus-search-local'),
      {
        hashed: true,
        language: ['zh', 'en'],
        indexBlog: false,
        indexDocs: true,
        docsRouteBasePath: '/',
        explicitSearchResultPath: true,
        highlightSearchTermsOnTargetPage: true,
      },
    ],
  ],
  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      metadata: [{ name: 'keywords', content: 'libcg, 2d graphics, cairo, canvas, rendering, rasterization, c' }],
      colorMode: {
        defaultMode: 'light',
        disableSwitch: false,
        respectPrefersColorScheme: true,
      },
      navbar: {
        title: 'libcg',
        items: [
          {
            type: 'docSidebar',
            sidebarId: 'guide',
            position: 'left',
            label: '入门指南',
          },
          {
            type: 'docSidebar',
            sidebarId: 'api',
            position: 'left',
            label: 'API 参考',
          },
          {
            type: 'docSidebar',
            sidebarId: 'examples',
            position: 'left',
            label: '使用示例',
          },
          {
            type: 'docSidebar',
            sidebarId: 'reference',
            position: 'left',
            label: '参考列表',
          },
          {
            type: 'localeDropdown',
            position: 'right',
          },
        ],
      },
      footer: {
        style: 'dark',
        copyright: `Copyright © ${new Date().getFullYear()} libcg. Built with Docusaurus.`,
      },
      prism: {
        theme: require('prism-react-renderer').themes.github,
        darkTheme: require('prism-react-renderer').themes.dracula,
        additionalLanguages: ['bash', 'c', 'cpp', 'json', 'makefile'],
      },
      tableOfContents: {
        minHeadingLevel: 2,
        maxHeadingLevel: 4,
      },
    }),
  i18n: {
    defaultLocale: 'zh',
    locales: ['zh', 'en'],
  },
};
