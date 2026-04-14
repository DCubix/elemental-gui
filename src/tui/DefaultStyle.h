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
},
"Switch": {
	"track": {
		"off": {
			"border": {
				"radius": 999.0,
				"color": [0.28, 0.28, 0.30, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.14, 0.14, 0.14, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.08, 0.08, 0.08, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"on": {
			"border": {
				"radius": 999.0,
				"color": [0.22, 0.40, 0.75, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.16, 0.30, 0.58, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.10, 0.22, 0.48, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		}
	},
	"thumb": {
		"normal": {
			"border": {
				"radius": 999.0,
				"color": [0.40, 0.40, 0.42, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.60, 0.60, 0.62, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.42, 0.42, 0.44, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"hover": {
			"border": {
				"radius": 999.0,
				"color": [0.50, 0.50, 0.52, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.70, 0.70, 0.72, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.52, 0.52, 0.54, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"click": {
			"border": {
				"radius": 999.0,
				"color": [0.35, 0.35, 0.37, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.48, 0.48, 0.50, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.32, 0.32, 0.34, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"normalOn": {
			"border": {
				"radius": 999.0,
				"color": [0.30, 0.50, 0.85, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.75, 0.82, 0.95, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.55, 0.65, 0.85, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"hoverOn": {
			"border": {
				"radius": 999.0,
				"color": [0.38, 0.58, 0.92, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.82, 0.88, 0.98, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.62, 0.72, 0.90, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		},
		"clickOn": {
			"border": {
				"radius": 999.0,
				"color": [0.22, 0.40, 0.72, 1.0],
				"width": 1.0
			},
			"background": {
				"gradient": {
					"startPos": [0.0, 0.0],
					"endPos": [0.0, 1.0],
					"colors": [
						{
							"color": [0.60, 0.70, 0.88, 1.0],
							"offset": 0.0
						},
						{
							"color": [0.42, 0.55, 0.78, 1.0],
							"offset": 1.0
						}
					]
				}
			}
		}
	}
},
"CheckBox": {
	"normal": {
		"border": {
			"radius": 4.0,
			"color": [0.28, 0.28, 0.32, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.14, 0.14, 0.14, 1.0],
						"offset": 0.0
					},
					{
						"color": [0.08, 0.08, 0.08, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"hover": {
		"border": {
			"radius": 4.0,
			"color": [0.30, 0.45, 0.78, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.16, 0.16, 0.19, 1.0],
						"offset": 0.0
					},
					{
						"color": [0.10, 0.10, 0.13, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"click": {
		"border": {
			"radius": 4.0,
			"color": [0.18, 0.28, 0.50, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.08, 0.08, 0.10, 1.0],
						"offset": 0.0
					},
					{
						"color": [0.05, 0.05, 0.07, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"checked": {
		"border": {
			"radius": 4.0,
			"color": [0.24, 0.42, 0.78, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.20, 0.35, 0.65, 1.0],
						"offset": 0.0
					},
					{
						"color": [0.14, 0.26, 0.52, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"checkedHover": {
		"border": {
			"radius": 4.0,
			"color": [0.32, 0.50, 0.88, 1.0],
			"width": 1.0
		},
		"background": {
			"gradient": {
				"startPos": [0.0, 0.0],
				"endPos": [0.0, 1.0],
				"colors": [
					{
						"color": [0.24, 0.42, 0.75, 1.0],
						"offset": 0.0
					},
					{
						"color": [0.18, 0.32, 0.62, 1.0],
						"offset": 1.0
					}
				]
			}
		}
	},
	"checkmark": {
		"stroke": {
			"color": [1.0, 1.0, 1.0, 1.0],
			"width": 2.0
		},
		"line-cap": "round",
		"line-join": "round",
		"svg": "M20 6 9 17l-5-5"
	}
},
"Menu": {
	"padding": {
		"horizontal": 0.0,
		"vertical": 4.0
	},
	"border": {
		"radius": 4.0,
		"color": [0.28, 0.28, 0.30, 1.0],
		"width": 1.0
	},
	"background": {
		"color": [0.16, 0.16, 0.16, 1.0]
	}
},
"MenuItem": {
	"padding": {
		"horizontal": 8.0,
		"vertical": 4.0
	},
	"normal": {
		"background": {
			"color": [0.0, 0.0, 0.0, 0.0]
		}
	},
	"hover": {
		"background": {
			"color": [0.20, 0.35, 0.65, 1.0]
		}
	},
	"click": {
		"background": {
			"color": [0.15, 0.28, 0.55, 1.0]
		}
	},
	"arrow": {
		"stroke": {
			"color": [1.0, 1.0, 1.0, 1.0],
			"width": 2.0
		},
		"line-cap": "round",
		"line-join": "round",
		"svg": "m9 18 6-6-6-6"
	}
},
"MenuSeparator": {
	"color": [0.28, 0.28, 0.30, 1.0],
	"margin": {
		"horizontal": 8.0,
		"vertical": 4.0
	}
}
})"
