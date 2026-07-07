/** @type {import('@docusaurus/plugin-content-docs').SidebarsConfig} */
const sidebars = {
  guide: [
    {
      type: 'category',
      label: '入门指南',
      link: { type: 'doc', id: 'guide/intro' },
      items: [
        'guide/intro',
        'guide/build-guide',
        'guide/getting-started',
      ],
    },
  ],
  api: [
    {
      type: 'category',
      label: 'API 参考',
      link: { type: 'doc', id: 'api/architecture' },
      items: [
        'api/architecture',
        'api/data-types',
        'api/matrix',
        'api/surface',
        'api/paint',
        'api/path',
        'api/context',
      ],
    },
  ],
  examples: [
    'examples/basic-shapes',
    'examples/fill-and-stroke',
    'examples/gradient',
    'examples/transform',
    'examples/clipping',
    'examples/texture',
    'examples/operators',
    'examples/mask',
  ],
  reference: [
    'reference/enumerations',
    'reference/operators',
    'reference/default-state',
  ],
};

module.exports = sidebars;
