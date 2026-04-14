R"({
"DefaultText": {
	"font": "Sans",
	"fontSize": 14.0,
	"fontSlant": "normal",
	"fontWeight": "normal",
	"color": [0.88, 0.88, 0.88, 1.0]
},
"DefaultTextBig": {
	"font": "Sans",
	"fontSize": 26.0,
	"fontSlant": "normal",
	"fontWeight": "bold",
	"color": [0.92, 0.92, 0.92, 1.0]
},
"Default": {
	"background": {
		"color": [0.12, 0.12, 0.12, 1.0]
	}
},
"Panel": {
	"border": {
		"radius": 0.0,
		"color": [0.25, 0.25, 0.25, 1.0],
		"width": 1.0
	},
	"background": {
		"gradient": {
			"startPos": [0.0, 0.0],
			"endPos": [1.0, 1.0],
			"colors": [
				{
					"color": [0.15, 0.15, 0.15, 1.0],
					"offset": 0.0
				},
				{
					"color":  [0.11, 0.11, 0.11, 1.0],
					"offset": 1.0
				}
			]
		}
	}
},
"Button": {
	"padding": {
		"horizontal": 12.0,
		"vertical": 6.0
	},
	"normal": {
		"border": {
			"radius": 2.0,
			"color": [0.22, 0.35, 0.65, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.18, 0.30, 0.55, 1.0],
						"offset": 0.4
					},
					{
						"color":  [0.12, 0.22, 0.45, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"hover": {
		"border": {
			"radius": 2.0,
			"color": [0.30, 0.45, 0.80, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.22, 0.38, 0.68, 1.0],
						"offset": 0.4
					},
					{
						"color":  [0.16, 0.30, 0.58, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"click": {
		"border": {
			"radius": 2.0,
			"color": [0.18, 0.28, 0.50, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.10, 0.18, 0.38, 1.0],
						"offset": 0.4
					},
					{
						"color":  [0.08, 0.14, 0.30, 1.0],
						"offset": 0.0
					}
				]
			}
		}
	}
},
"Scrollbar": {
	"track": {
		"border": {
			"radius": 999.0,
			"color": [0.25, 0.25, 0.25, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.10, 0.10, 0.10, 1.0],
						"offset": 0.4
					},
					{
						"color": [0.06, 0.06, 0.06, 1.0],
						"offset": 0.0
					}
				]
			}
		}
	},
	"thumb": {
		"normal": {
			"border": {
				"radius": 999.0,
				"color": [0.30, 0.30, 0.30, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.25, 0.25, 0.25, 1.0],
							"offset": 0.4
						},
						{
							"color": [0.18, 0.18, 0.18, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"hover": {
			"border": {
				"radius": 999.0,
				"color": [0.40, 0.40, 0.40, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.32, 0.32, 0.32, 1.0],
							"offset": 0.4
						},
						{
							"color": [0.24, 0.24, 0.24, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"click": {
			"border": {
				"radius": 999.0,
				"color": [0.35, 0.35, 0.35, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.20, 0.20, 0.20, 1.0],
							"offset": 0.4
						},
						{
							"color": [0.14, 0.14, 0.14, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		}
	}
},
"Slider": {
	"track": {
		"border": {
			"radius": 3.0,
			"color": [0.22, 0.22, 0.22, 1.0],
			"width": 1.0
		},
		"background": {
			"color": [0.08, 0.08, 0.08, 1.0]
		}
	},
	"thumb": {
		"normal": {
			"border": {
				"radius": 999.0,
				"color": [0.22, 0.35, 0.65, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.18, 0.30, 0.55, 1.0],
							"offset": 0.4
						},
						{
							"color": [0.12, 0.22, 0.45, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"hover": {
			"border": {
				"radius": 999.0,
				"color": [0.30, 0.45, 0.80, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.22, 0.38, 0.68, 1.0],
							"offset": 0.4
						},
						{
							"color": [0.16, 0.30, 0.58, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"click": {
			"border": {
				"radius": 999.0,
				"color": [0.18, 0.28, 0.50, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.10, 0.18, 0.38, 1.0],
							"offset": 0.4
						},
						{
							"color": [0.08, 0.14, 0.30, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		}
	}
},
"Edit": {
	"border": {
		"radius": 2.0,
		"color": [0.25, 0.25, 0.25, 1.0],
		"width": 1.0
	},
	"background": {
		"color": [0.08, 0.08, 0.08, 1.0]
	}
},
"Selection": {
	"background": {
		"color": [0.15, 0.40, 0.85, 0.45]
	}
}
})"
