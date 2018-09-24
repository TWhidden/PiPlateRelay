using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Reflection;

namespace PiRelayPlate.NetCore.Resources
{
    /// <summary>
    /// Provides access to embedded assembly files.
    /// </summary>
    internal static class EmbeddedResources
    {
        /// <summary>
        /// Initializes static members of the <see cref="EmbeddedResources"/> class.
        /// </summary>
        static EmbeddedResources()
        {
            ResourceNames = new ReadOnlyCollection<string>(Assembly.GetExecutingAssembly().GetManifestResourceNames());
        }

        /// <summary>
        /// Gets the resource names.
        /// </summary>
        /// <value>
        /// The resource names.
        /// </value>
        public static ReadOnlyCollection<string> ResourceNames { get; }

        /// <summary>
        /// Extracts all the file resources to the specified base path.
        /// </summary>
        public static void ExtractAll()
        {
            var basePath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

            foreach (var resourceName in ResourceNames)
            {
                var filename = resourceName.Substring($"{typeof(EmbeddedResources).Namespace}.".Length);
                var targetPath = Path.Combine(basePath, filename);
                if (File.Exists(targetPath))
                {
                    try
                    {
                        File.Delete(targetPath);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Could not delete resource {targetPath}. Error: {ex.Message}");
                    }
                }

                using (var stream = Assembly.GetExecutingAssembly()
                    .GetManifestResourceStream($"{typeof(EmbeddedResources).Namespace}.{filename}"))
                {
                    using (var outputStream = File.OpenWrite(targetPath))
                    {
                        stream?.CopyTo(outputStream);
                    }
                }

                try
                {
                    // copy to local user lib folder
                    targetPath = Path.Combine("/usr/lib", filename);
                    if (!File.Exists(targetPath))
                    {
                        using (var stream = Assembly.GetExecutingAssembly()
                            .GetManifestResourceStream($"{typeof(EmbeddedResources).Namespace}.{filename}"))
                        {
                            using (var outputStream = File.OpenWrite(targetPath))
                            {
                                stream?.CopyTo(outputStream);
                            }
                        }
                    }
                }
                catch(Exception ex)
                {
                    Console.WriteLine($"Could not write '{targetPath}'; Error: {ex.Message}");
                }
            }
        }
    }
}